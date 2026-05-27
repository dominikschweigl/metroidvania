#pragma once

#include "../../combat/hitbox.h"
#include "../base_entity.h"
#include <SFML/Graphics.hpp>
#include <optional>

class EnemyState;
class World;

// Abstract base for all enemies. Adds an enemy-side state machine on top of BaseEntity.
class BaseEnemy : public BaseEntity {
  public:
	// Template method: runs the state machine, applies gravity and collisions.
	void update(float deltaTime, const World &world, sf::Vector2f playerPos);

	// Hook called at the top of update(). Override to tick enemy-specific timers.
	virtual void onPreUpdate(float /*deltaTime*/) {}

	// Derived classes render with their own sprite set.
	virtual void draw(sf::RenderWindow &window) = 0;

	static constexpr int MAX_HEALTH = 5;

	EnemyState *getState() const { return currentState; }
	void setState(EnemyState *s) { currentState = s; }

	// Active damage rectangle for this frame, otherwise nullopt.
	[[nodiscard]] virtual std::optional<Hitbox> getHitbox() noexcept { return std::nullopt; }

	void collectHitboxes(std::vector<Hitbox> &hitboxes) override;

  protected:
	BaseEnemy(sf::Vector2f spawnPos, float entityWidth, float entityHeight)
	    : BaseEntity(spawnPos, entityWidth, entityHeight, MAX_HEALTH, Team::Enemy)
	{
	}

	virtual void applyGravity(float dt, const World &world);
	virtual bool isGroundBelow(const World &world) const;
	virtual float resolveHorizontal(float dt, const World &world);
	virtual float resolveVertical(float dt, const World &world);

	// State machine pointer. Derived class must assign an initial state
	EnemyState *currentState = nullptr;
};
