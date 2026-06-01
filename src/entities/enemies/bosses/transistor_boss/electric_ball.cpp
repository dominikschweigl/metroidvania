#include "electric_ball.h"
#include <cmath>

namespace transistor_boss {
namespace {
constexpr float BALL_PI = 3.14159265f;

// Deterministic flicker noise in [0, 1), used to crackle the sparks.
[[nodiscard]] float flickerNoise(float a, float b)
{
	const float v = std::sin(a * 12.9898f + b * 78.233f) * 43758.5453f;
	return v - std::floor(v);
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
	const float flicker = std::floor(age * 30.f);
	const float pulse = 0.5f + 0.5f * std::sin(age * 22.f);

	// Layered glow: faint cyan halo -> bright blue body -> near-white hot core.
	struct Layer {
		float radius;
		sf::Color color;
	};
	const Layer layers[] = {
	    {RADIUS * 1.9f, sf::Color{60, 150, 255, static_cast<std::uint8_t>(55.f + 45.f * pulse)}},
	    {RADIUS * 1.2f, sf::Color{120, 200, 255, 180}},
	    {RADIUS * 0.6f, sf::Color{235, 250, 255, 240}},
	};
	for (const Layer &layer : layers) {
		sf::CircleShape circle(layer.radius);
		circle.setPointCount(20);
		circle.setOrigin({layer.radius, layer.radius});
		circle.setPosition(position);
		circle.setFillColor(layer.color);
		window.draw(circle);
	}

	// A few short sparks crackling off the orb, re-rolled rapidly over time.
	constexpr int SPARKS = 5;
	for (int spark = 0; spark < SPARKS; ++spark) {
		const float angle = flickerNoise(static_cast<float>(spark), flicker) * 2.f * BALL_PI;
		const float length = RADIUS * (1.4f + 0.8f * flickerNoise(static_cast<float>(spark) + 0.5f, flicker));

		sf::VertexArray sparkLine(sf::PrimitiveType::Lines, 2);
		sparkLine[0].position = position;
		sparkLine[0].color = sf::Color{200, 240, 255, 220};
		sparkLine[1].position = {position.x + std::cos(angle) * length, position.y + std::sin(angle) * length};
		sparkLine[1].color = sf::Color{120, 200, 255, 0};
		window.draw(sparkLine);
	}
}

} // namespace transistor_boss
