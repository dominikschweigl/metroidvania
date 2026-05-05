#include "entity_physics.h"

namespace EntityPhysics {

bool isGroundBelow(sf::FloatRect bounds, const World &world)
{
	float bott = bounds.position.y + bounds.size.y;
	auto left = world.getTileAtCoordinate({bounds.position.x, bott + 1.f});
	auto right = world.getTileAtCoordinate({bounds.position.x + bounds.size.x, bott + 1.f});
	return (left.has_value() && left.value()->isSolid) || (right.has_value() && right.value()->isSolid);
}

void applyGravity(float &velY, bool &isOnGround, float dt, float gravity, sf::FloatRect bounds, const World &world)
{
	if (!isGroundBelow(bounds, world)) {
		isOnGround = false;
		velY += gravity * dt;
	}
}

float resolveHorizontal(sf::Vector2f pos, float &velX, float width, float height, float dt, const World &world)
{
	float deltaX = velX * dt;
	float futureX = pos.x + deltaX;
	sf::FloatRect bounds({pos.x - width / 2.f, pos.y - height}, {width, height});

	sf::FloatRect future({futureX - width / 2.f, bounds.position.y}, {width, height});
	if (world.isSolidAtRect(future)) {
		velX = 0.f;
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

float resolveVertical(sf::Vector2f pos, float &velY, bool &isOnGround, float width, float height, float dt,
                      const World &world)
{
	float deltaY = velY * dt;
	float futureY = pos.y + deltaY;
	sf::FloatRect bounds({pos.x - width / 2.f, pos.y - height}, {width, height});

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
		velY = 0.f;
	}
	return futureY;
}

void simulateMovement(float deltaTime, sf::Vector2f &position, sf::Vector2f &velocity, bool &isOnGround, float gravity,
                      float width, float height, const World &world)
{
	applyGravity(velocity.y, isOnGround, deltaTime, gravity,
	             sf::FloatRect({position.x - width / 2.f, position.y - height}, {width, height}), world);
	static constexpr float step = 0.01f;
	for (float t = 0.f; t < deltaTime; t += step) {
		position.x = resolveHorizontal(position, velocity.x, width, height, step, world);
		position.y = resolveVertical(position, velocity.y, isOnGround, width, height, step, world);
	}
}
} // namespace EntityPhysics
