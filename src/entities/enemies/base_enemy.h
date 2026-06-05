#pragma once

#include "../../combat/hitbox.h"
#include "../../items/item.h"
#include "../base_entity.h"
#include <SFML/Graphics.hpp>
#include <optional>
#include <vector>

class EnemyState;
class World;

// Abstract base for all enemies. Adds an enemy-side state machine on top of BaseEntity.
class BaseEnemy : public BaseEntity {
  public:
	// Template method: runs the state machine, applies gravity and collisions.
	void update(float deltaTime, const World &world, sf::Vector2f playerPos, sf::FloatRect playerBounds = {});

	// Hook called at the top of update(). Override to tick enemy-specific timers.
	virtual void onPreUpdate(float /*deltaTime*/) {}

	// Derived classes render with their own sprite set.
	virtual void draw(sf::RenderWindow &window) = 0;

	static constexpr int MAX_HEALTH = 5;

	EnemyState *getState() const { return currentState; }
	void setState(EnemyState *s) { currentState = s; }

	// Active damage rectangle for this frame, otherwise nullopt.
	[[nodiscard]] virtual std::optional<Hitbox> getHitbox() noexcept { return std::nullopt; }

	// Returns items to drop when this enemy dies. Override per enemy to configure drops.
	[[nodiscard]] virtual std::vector<std::unique_ptr<Item>> rollDrops() { return {}; }

	void collectHitboxes(std::vector<Hitbox> &hitboxes) override;

	json serialize() const override;

  protected:
	BaseEnemy(sf::Vector2f spawnPos, float entityWidth, float entityHeight, int maxHealth = MAX_HEALTH)
	    : BaseEntity(spawnPos, entityWidth, entityHeight, maxHealth, Team::Enemy)
	{
	}

	virtual void applyGravity(float dt, const World &world);
	virtual bool isGroundBelow(const World &world) const;
	virtual float resolveHorizontal(float dt, const World &world);
	virtual float resolveVertical(float dt, const World &world);

	// State machine pointer. Derived class must assign an initial state
	EnemyState *currentState = nullptr;

	sf::FloatRect lastPlayerBounds;

	// Most recent context passed to update(), cached so onPreUpdate() (which only
	// receives deltaTime) can drive sub-entities through their own update().
	// Pointer because there is no world yet before the first update() call.
	const World *lastWorld = nullptr;
	sf::Vector2f lastPlayerPos;
};
