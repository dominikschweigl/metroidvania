#pragma once

#include "../../world/world.h"
#include <SFML/Graphics.hpp>

class EnemyState;

// Abstract base for all enemies. Owns common physics, position, velocity and state machine
class BaseEnemy {
  public:
	enum class Direction { Left, Right };

	virtual ~BaseEnemy() = default;

	// Template method: runs the state machine, applies gravity and collisions.
	void update(float deltaTime, const World &world, sf::Vector2f playerPos);

	// Hook called at the top of update(). Override to tick enemy-specific timers.
	virtual void onPreUpdate(float /*deltaTime*/) {}

	// Derived classes render with their own sprite set.
	virtual void draw(sf::RenderWindow &window) = 0;

	// Shared geometry
	virtual sf::FloatRect getBounds() const { return {{pos.x - width / 2.f, pos.y - height}, {width, height}}; }

	sf::Vector2f getPosition() const { return pos; }
	sf::Vector2f getVelocity() const { return vel; }
	void setVelocity(sf::Vector2f v) { vel = v; }
	void setVelocityX(float vx) { vel.x = vx; }
	void setVelocityY(float vy) { vel.y = vy; }
	Direction getFacing() const { return facing; }
	void setFacing(Direction d) { facing = d; }
	bool isOnGroundFlag() const { return isOnGround; }
	void setOnGround(bool onGround) { isOnGround = onGround; }

  protected:
	BaseEnemy(sf::Vector2f spawnPos, float entityWidth, float entityHeight)
	    : pos(spawnPos), width(entityWidth), height(entityHeight)
	{
	}

	virtual void applyGravity(float dt, const World &world);
	virtual bool isGroundBelow(const World &world) const;
	virtual float resolveHorizontal(float dt, const World &world);
	virtual float resolveVertical(float dt, const World &world);

	// Shared physics state
	sf::Vector2f pos;
	sf::Vector2f vel{0.f, 0.f};
	bool isOnGround = false;
	Direction facing = Direction::Right;

	// Dimensions supplied by the concrete enemy at construction time.
	const float width;
	const float height;

	// State machine pointer. Derived class must assign an initial state
	EnemyState *currentState = nullptr;

	static constexpr float GRAVITY = 1200.f;

	friend struct EnemyTestAccess;
};
