#include "transistor_boss.h"
#include "../../../../core/audio_manager.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace {
constexpr float AURA_PI = 3.14159265f;

// Linear blend between two colors (component-wise, including alpha).
[[nodiscard]] sf::Color lerpColor(const sf::Color &from, const sf::Color &to, float t)
{
	const auto mix = [t](std::uint8_t a, std::uint8_t b) {
		return static_cast<std::uint8_t>(static_cast<float>(a) + (static_cast<float>(b) - static_cast<float>(a)) * t);
	};
	return sf::Color{mix(from.r, to.r), mix(from.g, to.g), mix(from.b, to.b), mix(from.a, to.a)};
}

// Cheap deterministic hash noise in [0, 1), used to jitter the lightning bolts.
[[nodiscard]] float hashNoise(float a, float b)
{
	const float v = std::sin(a * 12.9898f + b * 78.233f) * 43758.5453f;
	return v - std::floor(v);
}
} // namespace

TransistorBoss::TransistorBoss(sf::Vector2f spawnPos)
    : BaseEnemy(spawnPos, ENTITY_WIDTH, ENTITY_HEIGHT, BOSS_HEALTH),
      roamingTexture(AssetManager::getInstance().getTexture(TRANSISTOR_BOSS_ROAMING)),
      chargeAttackWindupTexture(AssetManager::getInstance().getTexture(TRANSISTOR_BOSS_CHARGE_ATTACK_WINDUP)),
      chargeAttackTexture(AssetManager::getInstance().getTexture(TRANSISTOR_BOSS_CHARGE_ATTACK)),
      recoverTexture(AssetManager::getInstance().getTexture(TRANSISTOR_BOSS_RECOVER)), sprite(roamingTexture)
{
	sprite.setOrigin({FRAME_SIZE / 2.f, static_cast<float>(FRAME_SIZE)});
	currentState = &states.roaming;
}

void TransistorBoss::draw(sf::RenderWindow &window)
{
	if (auraPhase != AuraPhase::None) {
		drawChargeAura(window);
	}

	sprite.setPosition(position);
	sprite.setScale({direction == Direction::Right ? 1.f : -1.f, 1.f});
	sprite.setColor(isHurtFlashing() ? sf::Color{255, 80, 80} : sf::Color::White);
	window.draw(sprite);

	for (const projectiles::ElectricBall &ball : electricBalls)
		ball.draw(window);
}

void TransistorBoss::drawChargeAura(sf::RenderWindow &window) const
{

	constexpr float PIXEL = 4.f;

	// Stepped (digital) pulse so the electricity flickers in discrete tiers
	// rather than fading smoothly — reads as "powered circuitry", not a soft glow.
	constexpr float PULSE_SPEED = 9.f;
	const float pulse = std::round((0.5f + 0.5f * std::sin(auraTimer * PULSE_SPEED)) * 4.f) / 4.f;

	// Each palette runs outer -> mid -> inner (core) for a multi-tone radial
	// look. Cold electric blue while charging up (telegraph), hot orange once it
	// is actively damaging.
	struct AuraPalette {
		sf::Color outer;
		sf::Color mid;
		sf::Color inner;
		sf::Color bolt;
	};
	const AuraPalette windup{{30, 70, 180}, {70, 150, 255}, {210, 240, 255}, {225, 245, 255}};
	const AuraPalette damage{{150, 40, 0}, {255, 120, 30}, {255, 240, 190}, {255, 235, 160}};
	const AuraPalette &palette = (auraPhase == AuraPhase::Windup) ? windup : damage;

	// position is the bottom-center of the body; lift to the body's center.
	const sf::Vector2f center{position.x, position.y - height / 2.f};
	const float maxRadius = CHARGE_AURA_RADIUS * (1.f + 0.06f * pulse);

	// All cells go into one batch. Each cell is a PIXEL-sized quad (two triangles).
	sf::VertexArray cells(sf::PrimitiveType::Triangles);
	const auto addCell = [&cells](float cellX, float cellY, sf::Color color) {
		const float half = PIXEL * 0.5f;
		const sf::Vector2f topLeft{cellX - half, cellY - half};
		const sf::Vector2f topRight{cellX + half, cellY - half};
		const sf::Vector2f bottomRight{cellX + half, cellY + half};
		const sf::Vector2f bottomLeft{cellX - half, cellY + half};
		cells.append({topLeft, color});
		cells.append({topRight, color});
		cells.append({bottomRight, color});
		cells.append({topLeft, color});
		cells.append({bottomRight, color});
		cells.append({bottomLeft, color});
	};

	// Snap a local offset to the pixel grid so bolts and core align with the glow.
	const auto snap = [](float value) { return std::round(value / PIXEL) * PIXEL; };

	// 1) Chunky radial glow: walk the local pixel grid and tint each cell by its
	// distance from the center, quantized into a few hard bands (no gradient).
	constexpr int GLOW_BANDS = 5;
	const int cellRange = static_cast<int>(maxRadius / PIXEL) + 1;
	for (int gridX = -cellRange; gridX <= cellRange; ++gridX) {
		for (int gridY = -cellRange; gridY <= cellRange; ++gridY) {
			const float offsetX = gridX * PIXEL;
			const float offsetY = gridY * PIXEL;
			const float dist = std::hypot(offsetX, offsetY);
			if (dist > maxRadius)
				continue;

			const float t = 1.f - dist / maxRadius; // 0 = edge, 1 = core
			const float band =
			    std::min(1.f, std::floor(t * GLOW_BANDS) / (GLOW_BANDS - 1)); // hard tiers
			sf::Color color = (band < 0.5f) ? lerpColor(palette.outer, palette.mid, band * 2.f)
			                                : lerpColor(palette.mid, palette.inner, (band - 0.5f) * 2.f);
			color.a = static_cast<std::uint8_t>((28.f + 95.f * band) * (0.7f + 0.3f * pulse));
			addCell(center.x + offsetX, center.y + offsetY, color);
		}
	}

	// 2) Chunky lightning bolts: sample a jittered radial line, snap each sample
	// to the grid, then fill a staircase of cells between samples so the bolt
	// reads as a connected blocky arc rather than dotted points.
	constexpr int BOLTS = 9;
	constexpr int SEGMENTS = 7;
	const float rotation = auraTimer * 0.7f;
	const float flicker = std::floor(auraTimer * 18.f); // re-rolls bolt shapes ~18x/sec
	const float innerRadius = maxRadius * 0.28f;
	const float outerRadius = maxRadius * 1.08f;

	for (int bolt = 0; bolt < BOLTS; ++bolt) {
		const float baseAngle = (static_cast<float>(bolt) / BOLTS) * 2.f * AURA_PI + rotation;

		std::array<sf::Vector2f, SEGMENTS + 1> points;
		for (int segment = 0; segment <= SEGMENTS; ++segment) {
			const float u = static_cast<float>(segment) / SEGMENTS; // 0 at core, 1 at tip
			const float radius = innerRadius + (outerRadius - innerRadius) * u;

			// Perpendicular wobble grows toward the tip for a forked look.
			const float wobble =
			    hashNoise(static_cast<float>(bolt) * 7.31f + static_cast<float>(segment), flicker) - 0.5f;
			const float angle = baseAngle + wobble * 0.9f * u;
			points[segment] = {snap(std::cos(angle) * radius), snap(std::sin(angle) * radius)};
		}

		for (int segment = 0; segment < SEGMENTS; ++segment) {
			const float u = static_cast<float>(segment) / SEGMENTS;
			const sf::Vector2f from = points[segment];
			const sf::Vector2f to = points[segment + 1];
			const int steps =
			    std::max(1, static_cast<int>(std::max(std::abs(to.x - from.x), std::abs(to.y - from.y)) / PIXEL));

			sf::Color color = palette.bolt;
			color.a = static_cast<std::uint8_t>(255.f * (1.f - 0.7f * u) * (0.55f + 0.45f * pulse));
			for (int step = 0; step <= steps; ++step) {
				const float f = static_cast<float>(step) / steps;
				const float cellX = snap(from.x + (to.x - from.x) * f);
				const float cellY = snap(from.y + (to.y - from.y) * f);
				addCell(center.x + cellX, center.y + cellY, color);
			}
		}
	}

	// 3) Bright hot core: a small snapped cluster of cells to anchor the effect.
	const float coreRadius = maxRadius * 0.13f;
	const int coreRange = static_cast<int>(coreRadius / PIXEL);
	sf::Color coreColor = palette.inner;
	coreColor.a = static_cast<std::uint8_t>(std::min(190.f, 120.f + 55.f * pulse));
	for (int gridX = -coreRange; gridX <= coreRange; ++gridX)
		for (int gridY = -coreRange; gridY <= coreRange; ++gridY)
			if (std::hypot(gridX * PIXEL, gridY * PIXEL) <= coreRadius)
				addCell(center.x + gridX * PIXEL, center.y + gridY * PIXEL, coreColor);

	window.draw(cells);
}

void TransistorBoss::onHit(const Hitbox & /*hit*/) noexcept
{
	triggerHurtFlash();
}

void TransistorBoss::onPreUpdate(float deltaTime)
{
	shootAttackCooldown = std::max(0.f, shootAttackCooldown - deltaTime);
	chargeAttackCooldown = std::max(0.f, chargeAttackCooldown - deltaTime);
	auraTimer += deltaTime;

	const sf::FloatRect playerBounds = lastPlayerBounds;
	std::erase_if(electricBalls, [this, deltaTime, playerBounds](projectiles::ElectricBall &ball) {
		if (ball.hasHitPlayer()) {
			endedBallSourceIds.push_back(ball.getSourceId());
			return true;
		}
		if (ball.update(deltaTime)) {
			endedBallSourceIds.push_back(ball.getSourceId());
			return true;
		}
		if (ball.getBounds().findIntersection(playerBounds).has_value())
			ball.markHitPlayer();
		return false;
	});
}

std::optional<Hitbox> TransistorBoss::getHitbox() noexcept
{
	if (auraPhase != AuraPhase::Damage)
		return std::nullopt;

	const sf::Vector2f center{position.x, position.y - height / 2.f};
	const sf::FloatRect bounds({center.x - CHARGE_AURA_RADIUS, center.y - CHARGE_AURA_RADIUS},
	                           {2.f * CHARGE_AURA_RADIUS, 2.f * CHARGE_AURA_RADIUS});
	return Hitbox{bounds, CHARGE_DAMAGE, Team::Enemy, chargeSourceId};
}

void TransistorBoss::spawnElectricBall(const sf::Vector2f targetPos)
{
	const sf::Vector2f origin{position.x, position.y - height / 2.f};
	sf::Vector2f aim = targetPos - origin;
	const float length = std::hypot(aim.x, aim.y);
	if (length > 0.0001f)
		aim /= length;
	else
		aim = {(direction == Direction::Right) ? 1.f : -1.f, 0.f};

	electricBalls.emplace_back(origin, aim);
	AudioManager::getInstance().playSound(SoundEffect::TRANSISTOR_BOSS_SHOOT_ATTACK);
}

void TransistorBoss::collectHitboxes(std::vector<Hitbox> &hitboxes)
{
	BaseEnemy::collectHitboxes(hitboxes);
	for (const projectiles::ElectricBall &ball : electricBalls)
		hitboxes.push_back(ball.getHitbox());
}

void TransistorBoss::drainEndedSourceIds(std::vector<std::uint32_t> &out)
{
	out.insert(out.end(), endedBallSourceIds.begin(), endedBallSourceIds.end());
	endedBallSourceIds.clear();
}

void TransistorBoss::setAnimation(TransistorBossAnimation anim, int frame)
{
	switch (anim) {
	case TransistorBossAnimation::Roaming:
		sprite.setTexture(roamingTexture);
		break;
	case TransistorBossAnimation::ChargeAttackWindup:
		sprite.setTexture(chargeAttackWindupTexture);
		break;
	case TransistorBossAnimation::ChargeAttack:
		sprite.setTexture(chargeAttackTexture);
		break;
	case TransistorBossAnimation::Recover:
		sprite.setTexture(recoverTexture);
		break;
	}
	sprite.setTextureRect(sf::IntRect({frame * FRAME_SIZE, 0}, {FRAME_SIZE, FRAME_SIZE}));
}