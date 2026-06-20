#include <catch2/catch_test_macros.hpp>

#include "entities/entity_physics.h"
#include "world/world.h"
#include <vector>

namespace {

static constexpr float T = World::TILE_SIZE;
static constexpr float PLAYER_SIZE = 32.f;
static constexpr float WALL_LEFT = 2 * T;
static constexpr float WALL_TOP = 1 * T;
static constexpr float FLOOR_TOP = 3 * T;

World makeEmptyWorld()
{
	std::vector<std::vector<int>> grid(10, std::vector<int>(10, 0));
	World world("test");
	world.loadFromGrid(grid);
	return world;
}

World makeWorldWithWallTile()
{
	std::vector<std::vector<int>> grid(10, std::vector<int>(10, 0));
	grid[1][2] = 1;
	World world("test");
	world.loadFromGrid(grid);
	return world;
}

World makeWorldWithFloor()
{
	std::vector<std::vector<int>> grid(10, std::vector<int>(10, 0));
	for (int col = 0; col < 10; ++col)
		grid[3][col] = 1;
	World world("test");
	world.loadFromGrid(grid);
	return world;
}

} // namespace

// ─── isGroundBelow ────────────────────────────────────────────────────────────

TEST_CASE("isGroundBelow: detects solid tile directly below entity")
{
	World world = makeWorldWithFloor();
	sf::FloatRect bounds({WALL_LEFT, FLOOR_TOP - PLAYER_SIZE}, {PLAYER_SIZE, PLAYER_SIZE});
	REQUIRE(EntityPhysics::isGroundBelow(bounds, world));
}

TEST_CASE("isGroundBelow: returns false when no ground below")
{
	World world = makeEmptyWorld();
	sf::FloatRect bounds({WALL_LEFT, FLOOR_TOP - PLAYER_SIZE}, {PLAYER_SIZE, PLAYER_SIZE});
	REQUIRE(!EntityPhysics::isGroundBelow(bounds, world));
}

TEST_CASE("isGroundBelow: returns false when entity is far above ground")
{
	World world = makeWorldWithFloor();
	sf::FloatRect bounds({WALL_LEFT, WALL_TOP}, {PLAYER_SIZE, PLAYER_SIZE});
	REQUIRE(!EntityPhysics::isGroundBelow(bounds, world));
}

// ─── isWallOnLeft / isWallOnRight ─────────────────────────────────────────────

TEST_CASE("isWallOnLeft: detects solid tile to the left")
{
	World world = makeWorldWithWallTile();
	sf::Vector2f pos{WALL_LEFT + T + PLAYER_SIZE / 2.f, WALL_TOP + T / 2.f};
	REQUIRE(EntityPhysics::isWallOnLeft(pos, PLAYER_SIZE, PLAYER_SIZE, world));
}

TEST_CASE("isWallOnLeft: returns false when nothing to the left")
{
	World world = makeEmptyWorld();
	sf::Vector2f pos{5 * T, WALL_TOP + T / 2.f};
	REQUIRE(!EntityPhysics::isWallOnLeft(pos, PLAYER_SIZE, PLAYER_SIZE, world));
}

TEST_CASE("isWallOnRight: detects solid tile to the right")
{
	World world = makeWorldWithWallTile();
	sf::Vector2f pos{WALL_LEFT - PLAYER_SIZE / 2.f, WALL_TOP + T / 2.f};
	REQUIRE(EntityPhysics::isWallOnRight(pos, PLAYER_SIZE, PLAYER_SIZE, world));
}

TEST_CASE("isWallOnRight: returns false when nothing to the right")
{
	World world = makeEmptyWorld();
	sf::Vector2f pos{5 * T, WALL_TOP + T / 2.f};
	REQUIRE(!EntityPhysics::isWallOnRight(pos, PLAYER_SIZE, PLAYER_SIZE, world));
}

// ─── applyGravity ─────────────────────────────────────────────────────────────

TEST_CASE("applyGravity: accelerates downward when no ground below")
{
	World world = makeEmptyWorld();
	float velY = 0.f;
	bool isOnGround = false;
	sf::FloatRect bounds({WALL_LEFT, WALL_TOP}, {PLAYER_SIZE, PLAYER_SIZE});
	EntityPhysics::applyGravity(velY, isOnGround, 0.016f, 800.f, bounds, world);
	REQUIRE(velY > 0.f);
	REQUIRE(!isOnGround);
}

TEST_CASE("applyGravity: does not accelerate when ground is directly below")
{
	World world = makeWorldWithFloor();
	float velY = 0.f;
	bool isOnGround = true;
	sf::FloatRect bounds({WALL_LEFT, FLOOR_TOP - PLAYER_SIZE}, {PLAYER_SIZE, PLAYER_SIZE});
	EntityPhysics::applyGravity(velY, isOnGround, 0.016f, 800.f, bounds, world);
	REQUIRE(velY == 0.f);
	REQUIRE(isOnGround);
}

TEST_CASE("applyGravity: sets isOnGround to true when ground is below even if previously false")
{
	World world = makeWorldWithFloor();
	float velY = 0.f;
	bool isOnGround = false;
	sf::FloatRect bounds({WALL_LEFT, FLOOR_TOP - PLAYER_SIZE}, {PLAYER_SIZE, PLAYER_SIZE});
	EntityPhysics::applyGravity(velY, isOnGround, 0.016f, 800.f, bounds, world);
	REQUIRE(isOnGround);
	REQUIRE(velY == 0.f);
}

// ─── resolveHorizontal ────────────────────────────────────────────────────────

TEST_CASE("resolveHorizontal: moves freely when no obstacle")
{
	World world = makeEmptyWorld();
	sf::Vector2f pos{5 * T, WALL_TOP + T / 2.f};
	float velX = 100.f;
	float result = EntityPhysics::resolveHorizontal(pos, velX, PLAYER_SIZE, PLAYER_SIZE, 0.016f, world);
	REQUIRE(result > pos.x);
	REQUIRE(velX == 100.f);
}

TEST_CASE("resolveHorizontal: stops and snaps to left of wall on right collision")
{
	World world = makeWorldWithWallTile();
	sf::Vector2f pos{WALL_LEFT - PLAYER_SIZE / 2.f - 4.f, WALL_TOP + T / 2.f};
	float velX = 500.f;
	float result = EntityPhysics::resolveHorizontal(pos, velX, PLAYER_SIZE, PLAYER_SIZE, 0.016f, world);
	REQUIRE(velX == 0.f);
	REQUIRE(result + PLAYER_SIZE / 2.f < WALL_LEFT);
}

TEST_CASE("resolveHorizontal: stops and snaps to right of wall on left collision")
{
	World world = makeWorldWithWallTile();
	sf::Vector2f pos{WALL_LEFT + T + PLAYER_SIZE / 2.f + 4.f, WALL_TOP + T / 2.f};
	float velX = -500.f;
	float result = EntityPhysics::resolveHorizontal(pos, velX, PLAYER_SIZE, PLAYER_SIZE, 0.016f, world);
	REQUIRE(velX == 0.f);
	REQUIRE(result - PLAYER_SIZE / 2.f >= WALL_LEFT + T);
}

// ─── resolveVertical ──────────────────────────────────────────────────────────

TEST_CASE("resolveVertical: falls freely when no ground")
{
	World world = makeEmptyWorld();
	sf::Vector2f pos{5 * T, 2 * T};
	float velY = 200.f;
	bool isOnGround = false;
	float result = EntityPhysics::resolveVertical(pos, velY, isOnGround, PLAYER_SIZE, PLAYER_SIZE, 0.016f, world);
	REQUIRE(result > pos.y);
	REQUIRE(velY == 200.f);
	REQUIRE(!isOnGround);
}

TEST_CASE("resolveVertical: landing sets isOnGround and snaps feet to floor surface")
{
	World world = makeWorldWithFloor();
	sf::Vector2f pos{WALL_LEFT, FLOOR_TOP - 1.f};
	float velY = 500.f;
	bool isOnGround = false;
	float result = EntityPhysics::resolveVertical(pos, velY, isOnGround, PLAYER_SIZE, PLAYER_SIZE, 0.016f, world);
	REQUIRE(isOnGround);
	REQUIRE(result == FLOOR_TOP);
	REQUIRE(velY == 0.f);
}

TEST_CASE("resolveVertical: head collision snaps below ceiling and zeros upward velocity")
{
	World world = makeWorldWithFloor();
	sf::Vector2f pos{WALL_LEFT, FLOOR_TOP + PLAYER_SIZE + 2.f};
	float velY = -500.f;
	bool isOnGround = false;
	float result = EntityPhysics::resolveVertical(pos, velY, isOnGround, PLAYER_SIZE, PLAYER_SIZE, 0.016f, world);
	REQUIRE(velY == 0.f);
	REQUIRE(result == FLOOR_TOP + PLAYER_SIZE);
}

TEST_CASE("resolveVertical: does not snap player downward at wall-corner top edge")
{
	World world = makeWorldWithWallTile();
	sf::Vector2f pos{WALL_LEFT + T / 2.f, WALL_TOP + 1.f};
	float velY = -1000.f;
	bool isOnGround = false;
	float result = EntityPhysics::resolveVertical(pos, velY, isOnGround, PLAYER_SIZE, PLAYER_SIZE, 0.001f, world);
	REQUIRE(result == WALL_TOP);
	REQUIRE(velY == -1000.f);
	REQUIRE(!isOnGround);
}

TEST_CASE("resolveVertical: sets isOnGround when stationary player overlaps ground tile")
{
	World world = makeWorldWithWallTile();
	sf::Vector2f pos{WALL_LEFT + T / 2.f, WALL_TOP + PLAYER_SIZE / 2.f};
	float velY = 0.f;
	bool isOnGround = false;
	float result = EntityPhysics::resolveVertical(pos, velY, isOnGround, PLAYER_SIZE, PLAYER_SIZE, 0.016f, world);
	REQUIRE(isOnGround);
	REQUIRE(result == WALL_TOP);
	REQUIRE(velY == 0.f);
}

// ─── simulateMovement ─────────────────────────────────────────────────────────

TEST_CASE("simulateMovement: entity falls under gravity and lands on floor")
{
	World world = makeWorldWithFloor();
	sf::Vector2f position{WALL_LEFT, T};
	sf::Vector2f velocity{0.f, 0.f};
	bool isOnGround = false;
	EntityPhysics::simulateMovement(0.5f, position, velocity, isOnGround, 800.f, PLAYER_SIZE, PLAYER_SIZE, world);
	REQUIRE(isOnGround);
	REQUIRE(position.y == FLOOR_TOP);
}

TEST_CASE("simulateMovement: entity is blocked horizontally by wall")
{
	World world = makeWorldWithWallTile();
	sf::Vector2f position{WALL_LEFT - PLAYER_SIZE / 2.f - 4.f, WALL_TOP + T / 2.f};
	sf::Vector2f velocity{1000.f, 0.f};
	bool isOnGround = false;
	EntityPhysics::simulateMovement(0.1f, position, velocity, isOnGround, 0.f, PLAYER_SIZE, PLAYER_SIZE, world);
	REQUIRE(velocity.x == 0.f);
	REQUIRE(position.x + PLAYER_SIZE / 2.f < WALL_LEFT);
}

TEST_CASE("simulateMovement: isOnGround recovers to true after being set false externally")
{
	World world = makeWorldWithFloor();
	sf::Vector2f position{WALL_LEFT, FLOOR_TOP};
	sf::Vector2f velocity{0.f, 0.f};
	bool isOnGround = false;
	EntityPhysics::simulateMovement(0.016f, position, velocity, isOnGround, 800.f, PLAYER_SIZE, PLAYER_SIZE, world);
	REQUIRE(isOnGround);
	REQUIRE(velocity.y == 0.f);
}
