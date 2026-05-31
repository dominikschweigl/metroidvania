#include "entity_physics.h"

namespace EntityPhysics {

bool isWallOnLeft(const sf::Vector2f position, const float width, const float height, const World &world)
{
	const float probeX = position.x - width / 2.f - 1.f;
	const auto top = world.getTileAtCoordinate({probeX, position.y - height + 1.f});
	const auto mid = world.getTileAtCoordinate({probeX, position.y - height / 2.f});
	return (top.has_value() && top.value()->isSolid) || (mid.has_value() && mid.value()->isSolid);
}

bool isWallOnRight(const sf::Vector2f position, const float width, const float height, const World &world)
{
	const float probeX = position.x + width / 2.f + 1.f;
	const auto top = world.getTileAtCoordinate({probeX, position.y - height + 1.f});
	const auto mid = world.getTileAtCoordinate({probeX, position.y - height / 2.f});
	return (top.has_value() && top.value()->isSolid) || (mid.has_value() && mid.value()->isSolid);
}

static bool isSolidTile(const tson::Tile *tile)
{
	if (!tile)
		return false;
	tson::Tile copy = *tile;
	// return copy.getProperties().getProperty("solid");
	return !copy.getObjectgroup().getObjects().empty();
}

static float tileLeft(const tson::Tile *tile, const World &world)
{
	// reconstruct x position from GID and map width
	const uint32_t gid = tile->getGid();
	const int mapWidth = world.getCurrentRoom()->width;
	const int localId = gid - tile->getTileset()->getFirstgid();
	const int x = localId % mapWidth;
	return float(x * World::TILE_SIZE);
}

bool isGroundBelow(sf::FloatRect bounds, const World &world)
{
	float bottom = bounds.position.y + bounds.size.y + 1.f;
	// Use isSolidAtRect with a thin strip below the entity
	sf::FloatRect probe({bounds.position.x, bottom}, {bounds.size.x, 1.f});
	return world.isSolidAtRect(probe);
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
	sf::FloatRect future({futureX - width / 2.f, pos.y - height}, {width, height});

	if (world.isSolidAtRect(future)) {
		velX = 0.f;
		// snap to tile boundary using grid position directly
		int tileX = static_cast<int>(pos.x / World::TILE_SIZE);
		if (deltaX >= 0.f)
			return float((tileX + 1) * World::TILE_SIZE) - width / 2.f - 1.f;
		return float(tileX * World::TILE_SIZE) + width / 2.f;
	}
	return futureX;
}

float resolveVertical(sf::Vector2f pos, float &velY, bool &isOnGround, float width, float height, float dt,
                      const World &world)
{
	float deltaY = velY * dt;
	float futureY = pos.y + deltaY;
	sf::FloatRect future({pos.x - width / 2.f, futureY - height}, {width, height});

	if (world.isSolidAtRect(future)) {
		if (deltaY > 0.f) {
			isOnGround = true;
			int tileY = static_cast<int>(futureY / World::TILE_SIZE);
			futureY = float(tileY * World::TILE_SIZE);
		} else if (deltaY < 0.f) {
			int tileY = static_cast<int>((futureY - height) / World::TILE_SIZE);
			futureY = float((tileY + 1) * World::TILE_SIZE) + height;
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
	float counter = 0.f;
	while (counter < deltaTime) {
		position.x = resolveHorizontal(position, velocity.x, width, height, step, world);
		position.y = resolveVertical(position, velocity.y, isOnGround, width, height, step, world);
		counter += step;
	}
}

} // namespace EntityPhysics
