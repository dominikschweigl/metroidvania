#include "transistor_boss_renderer.h"
#include "../../../../core/asset_manager.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace transistor_boss {

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

TransistorBossRenderer::TransistorBossRenderer()
    : roamingTexture(AssetManager::getInstance().getTexture(TRANSISTOR_BOSS_ROAMING)),
      chargeAttackWindupTexture(AssetManager::getInstance().getTexture(TRANSISTOR_BOSS_CHARGE_ATTACK_WINDUP)),
      chargeAttackTexture(AssetManager::getInstance().getTexture(TRANSISTOR_BOSS_CHARGE_ATTACK)),
      recoverTexture(AssetManager::getInstance().getTexture(TRANSISTOR_BOSS_RECOVER)),
      deathTexture(AssetManager::getInstance().getTexture(TRANSISTOR_BOSS_DEATH)), sprite(roamingTexture)
{
	sprite.setOrigin({FRAME_SIZE / 2.f, static_cast<float>(FRAME_SIZE)});
}

void TransistorBossRenderer::setAnimation(Animation anim, int frame)
{
	switch (anim) {
	case Animation::Roaming:
		sprite.setTexture(roamingTexture);
		break;
	case Animation::ChargeAttackWindup:
		sprite.setTexture(chargeAttackWindupTexture);
		break;
	case Animation::ChargeAttack:
		sprite.setTexture(chargeAttackTexture);
		break;
	case Animation::Recover:
		sprite.setTexture(recoverTexture);
		break;
	case Animation::Death:
		sprite.setTexture(deathTexture);
		break;
	}
	// Sheets are laid out as grids (kept under 1024px software-GL texture cap),
	// so row/column navigation is needed.
	const int framesPerRow = static_cast<int>(sprite.getTexture().getSize().x) / FRAME_SIZE;
	const int column = frame % framesPerRow;
	const int row = frame / framesPerRow;
	sprite.setTextureRect(sf::IntRect({column * FRAME_SIZE, row * FRAME_SIZE}, {FRAME_SIZE, FRAME_SIZE}));
}

void TransistorBossRenderer::drawSprite(sf::RenderWindow &window, const sf::Vector2f position, const float scaleX,
                                        const sf::Color tint)
{
	sprite.setPosition(position);
	sprite.setScale({scaleX, 1.f});
	sprite.setColor(tint);
	window.draw(sprite);
}

// Draws Aura during Windup and Charge Attack states. Is generated via SFML since no nice sprite could be found.
void TransistorBossRenderer::drawAura(sf::RenderWindow &window, const sf::Vector2f center, const AuraStyle style,
                                      const float timer, const float radius) const
{

	constexpr float PIXEL = 4.f;

	// Stepped (digital) pulse so the electricity flickers in discrete tiers
	// rather than fading smoothly — reads as "powered circuitry", not a soft glow.
	constexpr float PULSE_SPEED = 9.f;
	const float pulse = std::round((0.5f + 0.5f * std::sin(timer * PULSE_SPEED)) * 4.f) / 4.f;

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
	const AuraPalette &palette = (style == AuraStyle::Windup) ? windup : damage;

	const float maxRadius = radius * (1.f + 0.06f * pulse);

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

			const float t = 1.f - dist / maxRadius;
			const float band = std::min(1.f, std::floor(t * GLOW_BANDS) / (GLOW_BANDS - 1));
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
	const float rotation = timer * 0.7f;
	const float flicker = std::floor(timer * 18.f); // re-rolls bolt shapes ~18x/sec
	const float innerRadius = maxRadius * 0.28f;
	const float outerRadius = maxRadius * 1.08f;

	for (int bolt = 0; bolt < BOLTS; ++bolt) {
		const float baseAngle = (static_cast<float>(bolt) / BOLTS) * 2.f * AURA_PI + rotation;

		std::array<sf::Vector2f, SEGMENTS + 1> points;
		for (int segment = 0; segment <= SEGMENTS; ++segment) {
			const float u = static_cast<float>(segment) / SEGMENTS; // 0 at core, 1 at tip
			const float radiusAt = innerRadius + (outerRadius - innerRadius) * u;

			// Perpendicular wobble grows toward the tip for a forked look.
			const float wobble =
			    hashNoise(static_cast<float>(bolt) * 7.31f + static_cast<float>(segment), flicker) - 0.5f;
			const float angle = baseAngle + wobble * 0.9f * u;
			points[segment] = {snap(std::cos(angle) * radiusAt), snap(std::sin(angle) * radiusAt)};
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

// Draws Energy Beams that connect the boss to the Capacitor Minions in Stage 2.
// Is generated via SFML since no nice sprite could be found.
void TransistorBossRenderer::drawBeam(sf::RenderWindow &window, const sf::Vector2f from, const sf::Vector2f to,
                                      const float timer) const
{
	constexpr float PIXEL = 4.f;
	constexpr int SEGMENTS = 16;

	const sf::Vector2f delta = to - from;
	const float length = std::hypot(delta.x, delta.y);
	if (length < 1.f)
		return;

	// Unit perpendicular, used to jitter the bolt sideways and to flank it with glow.
	const sf::Vector2f normal{-delta.y / length, delta.x / length};
	const float flicker = std::floor(timer * 20.f); // re-rolls bolt shape

	const sf::Color glow{90, 170, 255};
	const sf::Color hot{235, 250, 255};

	const auto snap = [](float value) { return std::round(value / PIXEL) * PIXEL; };
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

	// Walk the segment, jittering each joint sideways (zero at the ends), and fill a
	// staircase of cells between joints so the beam reads as one connected blocky bolt.
	sf::Vector2f previous = from;
	for (int segment = 1; segment <= SEGMENTS; ++segment) {
		const float t = static_cast<float>(segment) / SEGMENTS;
		const float taper = std::sin(t * AURA_PI); // 0 at both ends, 1 mid-span
		const float wobble = (hashNoise(static_cast<float>(segment), flicker) - 0.5f) * 16.f * taper;
		const sf::Vector2f point = from + delta * t + normal * wobble;

		const sf::Vector2f start{snap(previous.x), snap(previous.y)};
		const sf::Vector2f end{snap(point.x), snap(point.y)};
		const int steps =
		    std::max(1, static_cast<int>(std::max(std::abs(end.x - start.x), std::abs(end.y - start.y)) / PIXEL));
		for (int step = 0; step <= steps; ++step) {
			const float f = static_cast<float>(step) / steps;
			const float cellX = snap(start.x + (end.x - start.x) * f);
			const float cellY = snap(start.y + (end.y - start.y) * f);
			sf::Color flank = glow;
			flank.a = 90;
			addCell(snap(cellX + normal.x * PIXEL), snap(cellY + normal.y * PIXEL), flank);
			addCell(snap(cellX - normal.x * PIXEL), snap(cellY - normal.y * PIXEL), flank);
			sf::Color coreColor = hot;
			coreColor.a = 235;
			addCell(cellX, cellY, coreColor);
		}
		previous = point;
	}

	window.draw(cells);
}

} // namespace transistor_boss
