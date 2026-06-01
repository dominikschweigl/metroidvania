#pragma once

#include "../../../../combat/hitbox.h"
#include <SFML/Graphics.hpp>
#include <cstdint>

namespace transistor_boss {

class ElectricBall {
  public:
	static constexpr float SPEED = 240.f;
	static constexpr float RADIUS = 9.f;
	static constexpr float LIFETIME = 2.5f;
	static constexpr int DAMAGE = 1;

	ElectricBall(sf::Vector2f startPos, sf::Vector2f direction);

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
	float age = 0.f;
	bool hitPlayer = false;
	std::uint32_t sourceId = nextSourceId();
};

} // namespace transistor_boss
