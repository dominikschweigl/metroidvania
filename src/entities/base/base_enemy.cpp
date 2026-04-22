#include "base_enemy.h"
#include "enemy_physics.h"
#include "enemy_state.h"

void BaseEnemy::update(float deltaTime, const World &world, sf::Vector2f playerPos)
{
	// Let concrete enemies tick their per-frame timers first.
	onPreUpdate(deltaTime);

	float deltaX = playerPos.x - pos.x;
	facing = (deltaX >= 0.f) ? Direction::Right : Direction::Left;

	if (currentState != nullptr) {
		EnemyState *nextState = currentState->update(deltaTime, *this, world, playerPos);
		if (nextState != currentState) {
			currentState->onExit(*this);
			nextState->onEnter(*this);
			currentState = nextState;
		}
		currentState->updateAnimation(deltaTime, *this);
	}

	applyGravity(deltaTime, world);
	pos.x = resolveHorizontal(deltaTime, world);
	pos.y = resolveVertical(deltaTime, world);
}

void BaseEnemy::applyGravity(float dt, const World &world)
{
	EnemyPhysics::applyGravity(vel.y, isOnGround, dt, gravity, getBounds(), world);
}

bool BaseEnemy::isGroundBelow(const World &world) const
{
	return EnemyPhysics::isGroundBelow(getBounds(), world);
}

float BaseEnemy::resolveHorizontal(float dt, const World &world)
{
	return EnemyPhysics::resolveHorizontal(pos, vel.x, width, height, dt, world);
}

float BaseEnemy::resolveVertical(float dt, const World &world)
{
	return EnemyPhysics::resolveVertical(pos, vel.y, isOnGround, width, height, dt, world);
}
