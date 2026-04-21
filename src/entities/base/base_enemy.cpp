#include "base_enemy.h"
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
	if (!isGroundBelow(world)) {
		isOnGround = false;
		vel.y += GRAVITY * dt;
	}
}

bool BaseEnemy::isGroundBelow(const World &world) const
{
	auto b = getBounds();
	float bott = b.position.y + b.size.y;
	auto left = world.getTileAtCoordinate({b.position.x, bott + 1.f});
	auto right = world.getTileAtCoordinate({b.position.x + b.size.x, bott + 1.f});
	return (left.has_value() && left.value()->isSolid) || (right.has_value() && right.value()->isSolid);
}

float BaseEnemy::resolveHorizontal(float dt, const World &world)
{
	float deltaX = vel.x * dt;
	float futureX = pos.x + deltaX;
	auto bounds = getBounds();

	sf::FloatRect future({futureX - width / 2.f, bounds.position.y}, {width, height});
	if (world.isSolidAtRect(future)) {
		vel.x = 0.f;
		auto tile = world.getTileAtCoordinate(pos);
		if (tile.has_value()) {
			if (deltaX >= 0.f) {
				return tile.value()->position.x + World::TILE_SIZE - width / 2.f - 1.f;
			}
			return tile.value()->position.x + width / 2.f;
		}
		return pos.x;
	}
	return futureX;
}

float BaseEnemy::resolveVertical(float dt, const World &world)
{
	float deltaY = vel.y * dt;
	float futureY = pos.y + deltaY;
	auto bounds = getBounds();

	sf::FloatRect future({bounds.position.x, futureY - height}, {width, height});
	if (world.isSolidAtRect(future)) {
		auto tile = world.getTileAtCoordinate(future.position);
		if (tile.has_value()) {
			if (deltaY > 0.f) {
				futureY = tile.value()->position.y + World::TILE_SIZE;
				isOnGround = true;
			} else if (deltaY < 0.f) {
				futureY = tile.value()->position.y + World::TILE_SIZE + height;
			}
		}
		vel.y = 0.f;
	}
	return futureY;
}
