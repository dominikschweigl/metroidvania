#pragma once

#include "../../projectiles/electric_ball.h"
#include "../base_enemy.h"
#include "states/flee_state.h"
#include "states/hover_state.h"
#include "states/shoot_state.h"
#include "states/swoop_state.h"

#include <SFML/Graphics.hpp>
#include <cstdint>
#include <vector>

// Flying enemy named "Capacitor". Fires energy shots at the player.
// Utilizes States Hover -> Shoot -> (Flee -> Swoop) -> Hover
class Capacitor : public BaseEnemy {
  public:
	static constexpr float ENTITY_WIDTH = 24.f;
	static constexpr float ENTITY_HEIGHT = 24.f;

	static constexpr int CAPACITOR_HEALTH = 3;

	static constexpr float MOVE_SPEED = 95.f;
	static constexpr float DETECT_RANGE = 360.f;
	static constexpr float STANDOFF_X = 110.f;
	static constexpr float HOVER_HEIGHT = 64.f;
	static constexpr float MAX_REACH_ABOVE = 96.f;
	static constexpr float ARRIVE_DEADZONE = 10.f;
	static constexpr float BOB_AMPLITUDE = 6.f;
	static constexpr float BOB_SPEED = 3.f;

	static constexpr float SHOOT_COOLDOWN = 1.8f;
	static constexpr float SHOOT_DUR = 0.3f;

	static constexpr float SHOT_SPEED = 150.f;
	static constexpr float SHOT_RADIUS = 6.f;

	static constexpr float FLEE_SPEED = 140.f;
	static constexpr float FLEE_DISTANCE = 150.f;
	static constexpr float FLEE_DUR = 0.6f;
	static constexpr float FLEE_RISE = 0.3f;

	static constexpr int SWOOP_AFTER_SHOTS = 3;
	static constexpr float SWOOP_SPEED = 230.f;
	static constexpr float SWOOP_STANDOFF = 14.f;
	static constexpr float SWOOP_HEIGHT = 22.f;
	static constexpr float SWOOP_ARRIVE = 12.f;
	static constexpr float SWOOP_LINGER = 0.35f;
	static constexpr float SWOOP_MAX_DUR = 2.0f;

	static constexpr int FRAME_SIZE = 32;

	struct States {
		capacitor::HoverState hover;
		capacitor::ShootState shoot;
		capacitor::FleeState flee;
		capacitor::SwoopState swoop;
	};
	States states;

	explicit Capacitor(sf::Vector2f spawnPos, float drop_chance);

	void draw(sf::RenderWindow &window) override;

	[[nodiscard]] sf::Vector2f getCenter() const noexcept;

	[[nodiscard]] sf::Vector2f hoverTarget(sf::Vector2f playerPos) const noexcept;

	// Close, low dive target next to the player used during the overcharge swoop.
	[[nodiscard]] sf::Vector2f swoopTarget(sf::Vector2f playerPos) const noexcept;

	void spawnShot(sf::Vector2f targetPos);

	[[nodiscard]] bool isShootOnCooldown() const noexcept { return shootCooldown > 0.f; }
	void startShootCooldown() noexcept { shootCooldown = SHOOT_COOLDOWN; }

	[[nodiscard]] bool shouldSwoop() const noexcept { return shotsSinceSwoop >= SWOOP_AFTER_SHOTS; }
	void resetSwoopCounter() noexcept { shotsSinceSwoop = 0; }
	[[nodiscard]] bool isSwooping() const noexcept { return currentState == &states.swoop; }

	[[nodiscard]] std::size_t shotCount() const noexcept { return shots.size(); }

	void collectHitboxes(std::vector<Hitbox> &hitboxes) override;
	void drainEndedSourceIds(std::vector<std::uint32_t> &out) override;

	json serialize() const override;

  protected:
	void onPreUpdate(float deltaTime) override;

  private:
	const sf::Texture &hoverTexture;
	sf::Sprite sprite;
	int frameCount = 1;
	int currentFrame = 0;
	float animTimer = 0.f;

	float shootCooldown = 0.f;
	float bobTimer = 0.f;
	int shotsSinceSwoop = 0;

	std::vector<projectiles::ElectricBall> shots;
	std::vector<std::uint32_t> endedShotSourceIds;
};
