#include "entity_physics.h"

namespace EntityPhysics {

// Small inset on perpendicular axis so corner tiles don't catch the sweep
constexpr float CORNER_INSET = 0.1f;

std::pair<int, int> tileRowRange(float top, float bottom)
{
	return {static_cast<int>(std::floor((top + CORNER_INSET) / World::TILE_SIZE)),
	        static_cast<int>(std::floor((bottom - CORNER_INSET) / World::TILE_SIZE))};
}

std::pair<int, int> tileColRange(float left, float right)
{
	return {static_cast<int>(std::floor((left + CORNER_INSET) / World::TILE_SIZE)),
	        static_cast<int>(std::floor((right - CORNER_INSET) / World::TILE_SIZE))};
}

bool isWallOnLeft(const sf::Vector2f position, const float width, const float height, const World &world)
{
	const sf::FloatRect probe({position.x - width / 2.f - 1.f, position.y - height + 1.f}, {1.f, height - 2.f});
	return world.isSolidAtRect(probe);
}

bool isWallOnRight(const sf::Vector2f position, const float width, const float height, const World &world)
{
	const sf::FloatRect probe({position.x + width / 2.f, position.y - height + 1.f}, {1.f, height - 2.f});
	return world.isSolidAtRect(probe);
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
	}
	if (!isOnGround) {
		velY += gravity * dt;
	}
}

SweepResult sweepHorizontal(sf::FloatRect body, float deltaX, const World &world)
{
	if (deltaX == 0.f)
		return {};

	const float left = body.position.x;
	const float right = body.position.x + body.size.x;
	const float top = body.position.y;
	const float bottom = body.position.y + body.size.y;
	auto [rowMin, rowMax] = tileRowRange(top, bottom);

	if (deltaX > 0.f) {
		// Leading edge: right.
		const int colStart = static_cast<int>(std::ceil(right / World::TILE_SIZE));
		const int colEnd = static_cast<int>(std::floor((right + deltaX) / World::TILE_SIZE));

		for (int col = colStart; col <= colEnd; ++col) {
			for (int row = rowMin; row <= rowMax; ++row) {
				if (world.isSolidTile(col, row)) {
					const float tileLeft = col * World::TILE_SIZE;
					return {true, (tileLeft - right) / deltaX};
				}
			}
		}
	} else {
		// Leading edge: left.
		const int colStart = static_cast<int>(std::floor(left / World::TILE_SIZE)) - 1;
		const int colEnd = static_cast<int>(std::floor((left + deltaX) / World::TILE_SIZE));

		for (int col = colStart; col >= colEnd; --col) {
			for (int row = rowMin; row <= rowMax; ++row) {
				if (world.isSolidTile(col, row)) {
					const float tileRight = (col + 1) * World::TILE_SIZE;
					return {true, (tileRight - left) / deltaX};
				}
			}
		}
	}

	return {};
}

SweepResult sweepVertical(sf::FloatRect body, float deltaY, const World &world)
{
	if (deltaY == 0.f)
		return {};

	const float left = body.position.x;
	const float right = body.position.x + body.size.x;
	const float top = body.position.y;
	const float bottom = body.position.y + body.size.y;
	auto [colMin, colMax] = tileColRange(left, right);

	if (deltaY > 0.f) {
		// Leading edge: bottom.
		const int rowStart = static_cast<int>(std::ceil(bottom / World::TILE_SIZE));
		const int rowEnd = static_cast<int>(std::floor((bottom + deltaY) / World::TILE_SIZE));

		for (int row = rowStart; row <= rowEnd; ++row) {
			for (int col = colMin; col <= colMax; ++col) {
				if (world.isSolidTile(col, row)) {
					const float tileTop = row * World::TILE_SIZE;
					return {true, (tileTop - bottom) / deltaY};
				}
			}
		}
	} else {
		// Leading edge: top.
		const int rowStart = static_cast<int>(std::floor(top / World::TILE_SIZE)) - 1;
		const int rowEnd = static_cast<int>(std::floor((top + deltaY) / World::TILE_SIZE));

		for (int row = rowStart; row >= rowEnd; --row) {
			for (int col = colMin; col <= colMax; ++col) {
				if (world.isSolidTile(col, row)) {
					const float tileBottom = (row + 1) * World::TILE_SIZE;
					return {true, (tileBottom - top) / deltaY};
				}
			}
		}
	}

	return {};
}

float resolveHorizontal(sf::Vector2f pos, float &velX, float width, float height, float dt, const World &world)
{
	if (velX == 0.f)
		return pos.x;

	const float deltaX = velX * dt;
	const sf::FloatRect body({pos.x - width / 2.f, pos.y - height}, {width, height});
	const auto result = sweepHorizontal(body, deltaX, world);

	if (result.hit)
		velX = 0.f;

	return pos.x + deltaX * result.fraction;
}

float resolveVertical(sf::Vector2f pos, float &velY, bool &isOnGround, float width, float height, float dt,
                      const World &world)
{
	if (velY == 0.f)
		return pos.y;

	const float deltaY = velY * dt;
	const sf::FloatRect body({pos.x - width / 2.f, pos.y - height}, {width, height});
	const auto result = sweepVertical(body, deltaY, world);

	if (result.hit) {
		if (deltaY > 0.f)
			isOnGround = true;
		velY = 0.f;
	}

	return pos.y + deltaY * result.fraction;
}

void simulateMovement(float deltaTime, sf::Vector2f &position, sf::Vector2f &velocity, bool &isOnGround, float gravity,
                      float width, float height, const World &world)
{
	float counter = 0.f;
	while (counter < deltaTime) {
		float s = std::min(EntityPhysics::PHYSICS_STEP, deltaTime - counter);
		position.x = resolveHorizontal(position, velocity.x, width, height, s, world);
		position.y = resolveVertical(position, velocity.y, isOnGround, width, height, s, world);
		applyGravity(velocity.y, isOnGround, s, gravity,
		             sf::FloatRect({position.x - width / 2.f, position.y - height}, {width, height}), world);
		counter += s;
	}
}

} // namespace EntityPhysics
