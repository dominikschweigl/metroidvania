#include "base_enemy.h"
#include "enemy_state.h"
#include "entity_physics.h"

void BaseEnemy::update(float deltaTime, const World &world,
					   sf::Vector2f playerPos) {
	// Let concrete enemies tick their per-frame timers first.
	onPreUpdate(deltaTime);

	float deltaX = playerPos.x - pos.x;
	facing = (deltaX >= 0.f) ? Direction::Right : Direction::Left;

	if (currentState != nullptr) {
		EnemyState *nextState =
			currentState->update(deltaTime, *this, world, playerPos);
		if (nextState != currentState) {
			currentState->onExit(*this);
			nextState->onEnter(*this);
			currentState = nextState;
		}
		currentState->updateAnimation(deltaTime, *this);
	}

	EntityPhysics::simulateMovement(deltaTime, pos, vel, isOnGround, gravity,
									width, height, world);
}

void BaseEnemy::applyGravity(float dt, const World &world) {
	EntityPhysics::applyGravity(vel.y, isOnGround, dt, gravity, getBounds(),
								world);
}

bool BaseEnemy::isGroundBelow(const World &world) const {
	return EntityPhysics::isGroundBelow(getBounds(), world);
}

float BaseEnemy::resolveHorizontal(float dt, const World &world) {
	return EntityPhysics::resolveHorizontal(pos, vel.x, width, height, dt,
											world);
}

float BaseEnemy::resolveVertical(float dt, const World &world) {
	return EntityPhysics::resolveVertical(pos, vel.y, isOnGround, width, height,
										  dt, world);
}
