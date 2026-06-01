#include "transistor_boss.h"
#include <algorithm>
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

	for (const transistor_boss::ElectricBall &ball : electricBalls)
		ball.draw(window);
}

void TransistorBoss::drawChargeAura(sf::RenderWindow &window) const
{
	// Pulse the electricity so the warning area reads as "live" rather than static.
	constexpr float PULSE_SPEED = 9.f;
	const float pulse = 0.5f + 0.5f * std::sin(auraTimer * PULSE_SPEED);

	// Each palette runs outer -> mid -> inner (core) so the glow reads as a
	// multi-tone radial gradient, not a flat disc. Cold electric blue while the
	// boss is charging up (telegraph), hot orange once it is actively damaging.
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

	// 1) Layered glow: large faint outer rings fading into a bright hot core.
	constexpr int GLOW_LAYERS = 7;
	for (int layer = 0; layer < GLOW_LAYERS; ++layer) {
		const float t = static_cast<float>(layer) / (GLOW_LAYERS - 1); // 0 = edge, 1 = core
		const float radius = maxRadius * (1.f - 0.6f * t);

		sf::Color color = (t < 0.5f) ? lerpColor(palette.outer, palette.mid, t * 2.f)
		                             : lerpColor(palette.mid, palette.inner, (t - 0.5f) * 2.f);
		color.a = static_cast<std::uint8_t>((25.f + 150.f * t) * (0.7f + 0.3f * pulse));

		sf::CircleShape glow(radius);
		glow.setPointCount(48);
		glow.setOrigin({radius, radius});
		glow.setPosition(center);
		glow.setFillColor(color);
		window.draw(glow);
	}

	// 2) Crackling lightning bolts radiating outward, jittered over time so they
	// flicker and fork like real electricity.
	constexpr int BOLTS = 9;
	constexpr int SEGMENTS = 7;
	const float rotation = auraTimer * 0.7f;
	const float flicker = std::floor(auraTimer * 18.f); // re-rolls bolt shapes ~18x/sec
	const float innerRadius = maxRadius * 0.28f;
	const float outerRadius = maxRadius * 1.08f;

	for (int bolt = 0; bolt < BOLTS; ++bolt) {
		const float baseAngle = (static_cast<float>(bolt) / BOLTS) * 2.f * AURA_PI + rotation;

		sf::VertexArray strip(sf::PrimitiveType::LineStrip, SEGMENTS + 1);
		for (int segment = 0; segment <= SEGMENTS; ++segment) {
			const float u = static_cast<float>(segment) / SEGMENTS; // 0 at core, 1 at tip
			const float radius = innerRadius + (outerRadius - innerRadius) * u;

			// Perpendicular wobble grows toward the tip for a forked look.
			const float wobble =
			    hashNoise(static_cast<float>(bolt) * 7.31f + static_cast<float>(segment), flicker) - 0.5f;
			const float angle = baseAngle + wobble * 0.9f * u;

			sf::Color color = palette.bolt;
			color.a = static_cast<std::uint8_t>(255.f * (1.f - 0.7f * u) * (0.55f + 0.45f * pulse));
			strip[segment].position = {center.x + std::cos(angle) * radius, center.y + std::sin(angle) * radius};
			strip[segment].color = color;
		}
		window.draw(strip);
	}

	// 3) Bright hot core to anchor the whole effect.
	const float coreRadius = maxRadius * 0.18f;
	sf::CircleShape core(coreRadius);
	core.setPointCount(24);
	core.setOrigin({coreRadius, coreRadius});
	core.setPosition(center);
	sf::Color coreColor = palette.inner;
	coreColor.a = static_cast<std::uint8_t>(180.f + 60.f * pulse);
	core.setFillColor(coreColor);
	window.draw(core);
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
	std::erase_if(electricBalls, [this, deltaTime, playerBounds](transistor_boss::ElectricBall &ball) {
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
}

void TransistorBoss::collectHitboxes(std::vector<Hitbox> &hitboxes)
{
	BaseEnemy::collectHitboxes(hitboxes);
	for (const transistor_boss::ElectricBall &ball : electricBalls)
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