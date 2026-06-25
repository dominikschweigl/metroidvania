#include "base_enemy.h"
#include "../../utils/EnemyStateFactory.hpp"
#include "../entity_physics.h"
#include "enemy_state.h"

void BaseEnemy::update(float deltaTime, const World &world, sf::Vector2f playerPos, sf::FloatRect playerBounds)
{
	tickHurtTimers(deltaTime);

	lastPlayerBounds = playerBounds;
	lastWorld = &world;
	lastPlayerPos = playerPos;

	// Let concrete enemies tick their per-frame timers first.
	onPreUpdate(deltaTime);

	const float deltaX = playerPos.x - position.x;
	direction = (deltaX >= 0.f) ? Direction::Right : Direction::Left;

	if (currentState != nullptr) {
		// While knocked back, freeze the state machine so it can't overwrite the
		// knockback velocity.
		if (!isKnockedBack()) {
			EnemyState *nextState = currentState->update(deltaTime, *this, world, playerPos);
			if (nextState != currentState) {
				currentState->onExit(*this);
				nextState->onEnter(*this);
				currentState = nextState;
			}
		}
		currentState->updateAnimation(deltaTime, *this);
	}

	EntityPhysics::simulateMovement(deltaTime, position, velocity, isOnGround, gravity, width, height, world);
}

void BaseEnemy::applyGravity(float dt, const World &world)
{
	EntityPhysics::applyGravity(velocity.y, isOnGround, dt, gravity, getBounds(), world);
}

bool BaseEnemy::isGroundBelow(const World &world) const
{
	return EntityPhysics::isGroundBelow(getBounds(), world);
}

float BaseEnemy::resolveHorizontal(float dt, const World &world)
{
	return EntityPhysics::resolveHorizontal(position, velocity.x, width, height, dt, world);
}

float BaseEnemy::resolveVertical(float dt, const World &world)
{
	return EntityPhysics::resolveVertical(position, velocity.y, isOnGround, width, height, dt, world);
}

void BaseEnemy::collectHitboxes(std::vector<Hitbox> &hitboxes)
{
	if (const auto hit = getHitbox())
		hitboxes.push_back(*hit);
}

std::vector<std::unique_ptr<Item>> BaseEnemy::rollDrops()
{
	std::bernoulli_distribution dist(drop_chance);
	std::vector<std::unique_ptr<Item>> drops;
	for (auto &item : drop_items) {
		if (item && dist(rng))
			drops.push_back(std::move(item));
	}
	drop_items.erase(std::remove(drop_items.begin(), drop_items.end(), nullptr), drop_items.end());
	return drops;
}

json BaseEnemy::serialize() const
{
	json j = BaseEntity::serialize();

	j["state"] = currentState->serialize();

	return j;
}

void BaseEnemy::deserialize(const json &j)
{
	BaseEntity::deserialize(j);

	if (j.contains("currentState")) {
		currentState = EnemyStateFactory::create(j["currentState"]);
	}
}
