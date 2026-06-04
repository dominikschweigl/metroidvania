#pragma once
#include "../../../combat/hitbox.h"
#include "../../../world/world.h"
#include "../../direction.h"
#include <SFML/Graphics.hpp>
#include <cstdint>

class HatProjectile {
  public:
	static constexpr float HAT_SPEED = 350.f;
	static constexpr float RETURN_SPEED = 500.f;
	static constexpr float BASE_TRAVEL = 200.f;
	static constexpr float PLAYER_VELOCITY_TRAVEL_INCREASE_FACTOR = 0.5f;
	static constexpr float MAX_TRAVEL_EPSILON = 1.f;
	static constexpr float CATCH_RADIUS = 5.f;
	static constexpr int DAMAGE = 1;
	static constexpr int SPIN_FRAME_COUNT = 7;
	static constexpr float SPIN_FRAME_DUR = 0.1f;
	static constexpr int FRAME_SIZE = 16;

	enum class Phase { Flying, Returning };

	HatProjectile(sf::Vector2f startPos, Direction direction, sf::Vector2f playerVelocity, const sf::Texture &texture);
	~HatProjectile() = default;

	// Returns true when the player catches the returning hat.
	bool update(float dt, sf::Vector2f playerPos, const World &world);
	void draw(sf::RenderWindow &window) const;

	[[nodiscard]] sf::FloatRect getBounds() const noexcept;

	// Active damage rectangle for the lifetime of the projectile. CombatSystem
	// currently ensures each enemy takes damage at most once per throw via sourceId.
	[[nodiscard]] Hitbox getHitbox() const noexcept { return Hitbox{getBounds(), DAMAGE, Team::Player, sourceId}; }

	[[nodiscard]] std::uint32_t getSourceId() const noexcept { return sourceId; }

  private:
	Phase phase = Phase::Flying;
	sf::Vector2f startPos;
	sf::Vector2f pos;
	sf::Vector2f velocity;
	float maxTravel;
	float distanceTraveled = 0.f;
	int spinFrame = 0;
	float spinTimer = 0.f;
	sf::Sprite sprite;
	std::uint32_t sourceId = nextSourceId();
};
