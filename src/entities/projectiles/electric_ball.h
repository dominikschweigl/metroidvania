#pragma once

#include "../../combat/hitbox.h"
#include <SFML/Graphics.hpp>
#include <cstdint>

namespace projectiles {

class ElectricBall {
  public:
	static constexpr float DEFAULT_SPEED = 240.f;
	static constexpr float DEFAULT_RADIUS = 9.f;
	static constexpr float LIFETIME = 2.5f;
	static constexpr int DAMAGE = 1;

	ElectricBall(sf::Vector2f startPos, sf::Vector2f direction, float speed = DEFAULT_SPEED,
	             float radius = DEFAULT_RADIUS);

	[[nodiscard]] bool update(float deltaTime);
	void draw(sf::RenderWindow &window) const;

	[[nodiscard]] sf::FloatRect getBounds() const noexcept;
	[[nodiscard]] Hitbox getHitbox() const noexcept { return Hitbox{getBounds(), DAMAGE, Team::Enemy, sourceId}; }
	[[nodiscard]] std::uint32_t getSourceId() const noexcept { return sourceId; }

	[[nodiscard]] bool hasHitPlayer() const noexcept { return hitPlayer; }
	void markHitPlayer() noexcept { hitPlayer = true; }

  private:
	sf::Vector2f position;
	sf::Vector2f velocity;
	float radius;
	float age = 0.f;
	bool hitPlayer = false;
	std::uint32_t sourceId = nextSourceId();
};

} // namespace projectiles
