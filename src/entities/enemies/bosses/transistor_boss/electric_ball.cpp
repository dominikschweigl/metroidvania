#include "electric_ball.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace transistor_boss {
namespace {
constexpr float BALL_PI = 3.14159265f;

// Deterministic flicker noise in [0, 1), used to crackle the sparks.
[[nodiscard]] float flickerNoise(float a, float b)
{
	const float v = std::sin(a * 12.9898f + b * 78.233f) * 43758.5453f;
	return v - std::floor(v);
}

// Linear blend between two colors (component-wise, including alpha).
[[nodiscard]] sf::Color lerpColor(const sf::Color &from, const sf::Color &to, float t)
{
	const auto mix = [t](std::uint8_t a, std::uint8_t b) {
		return static_cast<std::uint8_t>(static_cast<float>(a) + (static_cast<float>(b) - static_cast<float>(a)) * t);
	};
	return sf::Color{mix(from.r, to.r), mix(from.g, to.g), mix(from.b, to.b), mix(from.a, to.a)};
}
} // namespace

ElectricBall::ElectricBall(const sf::Vector2f startPos, const sf::Vector2f direction)
    : position(startPos), velocity(direction * SPEED)
{
}

bool ElectricBall::update(const float deltaTime)
{
	age += deltaTime;
	position += velocity * deltaTime;
	return age >= LIFETIME;
}

sf::FloatRect ElectricBall::getBounds() const noexcept
{
	return {{position.x - RADIUS, position.y - RADIUS}, {2.f * RADIUS, 2.f * RADIUS}};
}

void ElectricBall::draw(sf::RenderWindow &window) const
{

	constexpr float PIXEL = 2.f;

	// Stepped (digital) pulse so the orb flickers in discrete tiers, like the
	// boss's charge aura, rather than fading smoothly.
	const float pulse = std::round((0.5f + 0.5f * std::sin(age * 22.f)) * 4.f) / 4.f;
	const float flicker = std::floor(age * 30.f); // re-rolls spark shapes ~30x/sec

	// Faint cyan halo -> bright blue body -> near-white hot core.
	const sf::Color halo{60, 150, 255};
	const sf::Color body{120, 200, 255};
	const sf::Color core{235, 250, 255};
	const sf::Color sparkColor{200, 240, 255};

	const float maxRadius = RADIUS * 1.9f * (1.f + 0.05f * pulse);

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
	const auto snap = [](float value) { return std::round(value / PIXEL) * PIXEL; };

	// 1) Chunky radial glow: tint each grid cell by distance from the center,
	// quantized into a few hard bands (no smooth gradient).
	constexpr int GLOW_BANDS = 4;
	const int cellRange = static_cast<int>(maxRadius / PIXEL) + 1;
	for (int gridX = -cellRange; gridX <= cellRange; ++gridX) {
		for (int gridY = -cellRange; gridY <= cellRange; ++gridY) {
			const float offsetX = gridX * PIXEL;
			const float offsetY = gridY * PIXEL;
			const float dist = std::hypot(offsetX, offsetY);
			if (dist > maxRadius)
				continue;

			const float t = 1.f - dist / maxRadius; // 0 = edge, 1 = core
			const float band = std::min(1.f, std::floor(t * GLOW_BANDS) / (GLOW_BANDS - 1));
			sf::Color color =
			    (band < 0.5f) ? lerpColor(halo, body, band * 2.f) : lerpColor(body, core, (band - 0.5f) * 2.f);
			color.a = static_cast<std::uint8_t>((40.f + 150.f * band) * (0.7f + 0.3f * pulse));
			addCell(position.x + offsetX, position.y + offsetY, color);
		}
	}

	// 2) Chunky sparks crackling off the orb: snap a jittered radial line and
	// fill a staircase of cells so each spark reads as a connected blocky bolt.
	constexpr int SPARKS = 5;
	const float innerRadius = maxRadius * 0.2f;
	for (int spark = 0; spark < SPARKS; ++spark) {
		const float angle = flickerNoise(static_cast<float>(spark), flicker) * 2.f * BALL_PI;
		const float length = RADIUS * (1.4f + 0.8f * flickerNoise(static_cast<float>(spark) + 0.5f, flicker));

		const sf::Vector2f from{snap(std::cos(angle) * innerRadius), snap(std::sin(angle) * innerRadius)};
		const sf::Vector2f to{snap(std::cos(angle) * (innerRadius + length)),
		                      snap(std::sin(angle) * (innerRadius + length))};
		const int steps =
		    std::max(1, static_cast<int>(std::max(std::abs(to.x - from.x), std::abs(to.y - from.y)) / PIXEL));

		for (int step = 0; step <= steps; ++step) {
			const float f = static_cast<float>(step) / steps;
			const float cellX = snap(from.x + (to.x - from.x) * f);
			const float cellY = snap(from.y + (to.y - from.y) * f);
			sf::Color color = sparkColor;
			color.a = static_cast<std::uint8_t>(220.f * (1.f - 0.6f * f)); // fade toward the tip
			addCell(position.x + cellX, position.y + cellY, color);
		}
	}

	window.draw(cells);
}

} // namespace transistor_boss
