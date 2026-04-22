#include <catch2/catch_test_macros.hpp>

#include "enemy_test_access.h"

namespace {

// Minimal concrete enemy so the abstract BaseEnemy can be instantiated.
// Overrides `draw` as a no-op so tests never render.
class TestEnemy : public BaseEnemy {
  public:
	TestEnemy(sf::Vector2f spawn, float w = 32.f, float h = 32.f) : BaseEnemy(spawn, w, h) {}

	void draw(sf::RenderWindow &window) override {}

	int preUpdateCalls = 0;
	float lastPreUpdateDt = 0.f;

  protected:
	void onPreUpdate(float dt) override
	{
		++preUpdateCalls;
		lastPreUpdateDt = dt;
	}
};

// Probe state records invocations, forces a transition on the next update.
class TestState : public EnemyState {
  public:
	EnemyState *nextState = nullptr; // if non-null and != this, returned on update
	int updateCalls = 0;
	int animationCalls = 0;
	int enterCalls = 0;
	int exitCalls = 0;

	EnemyState *update(float dt, BaseEnemy &e, const World &w, sf::Vector2f p) override
	{
		++updateCalls;
		return (nextState != nullptr) ? nextState : this;
	}
	void updateAnimation(float dt, BaseEnemy &e) override { ++animationCalls; }
	void onEnter(BaseEnemy &e) override { ++enterCalls; }
	void onExit(BaseEnemy &e) override { ++exitCalls; }
};

} // namespace

namespace {

// 10 wide x 5 tall room, open interior, solid ring of floor/walls/ceiling.
// Tile coords (x, y) so y grows downward.
World makeWalledWorld()
{
	std::vector<std::vector<int>> g(5, std::vector<int>(10, 0));
	for (int x = 0; x < 10; ++x) {
		g[0][x] = 1; // ceiling
		g[4][x] = 1; // floor
	}
	for (int y = 0; y < 5; ++y) {
		g[y][0] = 1; // left wall
		g[y][9] = 1; // right wall
	}
	World w;
	w.loadFromGrid(g);
	return w;
}

// Fully open 10x5 world, no collisions anywhere.
World makeEmptyWorld()
{
	std::vector<std::vector<int>> g(5, std::vector<int>(10, 0));
	World w;
	w.loadFromGrid(g);
	return w;
}

constexpr float TILE = World::TILE_SIZE;

} // namespace

TEST_CASE("BaseEnemy initializes with given position and defaults")
{
	TestEnemy e({100.f, 200.f}, 28.f, 28.f);
	REQUIRE(e.getPosition() == sf::Vector2f{100.f, 200.f});
	REQUIRE(e.getVelocity() == sf::Vector2f{0.f, 0.f});
	REQUIRE(e.getFacing() == BaseEnemy::Direction::Right);
	REQUIRE_FALSE(e.isOnGroundFlag());
}

TEST_CASE("BaseEnemy::getBounds returns foot-centered rectangle")
{
	TestEnemy e({100.f, 200.f}, 28.f, 40.f);
	auto b = e.getBounds();
	REQUIRE(b.position.x == 100.f - 14.f); // pos.x - width/2
	REQUIRE(b.position.y == 200.f - 40.f); // pos.y - height
	REQUIRE(b.size.x == 28.f);
	REQUIRE(b.size.y == 40.f);
}

TEST_CASE("isGroundBelow reports true when enemy stands on a solid tile")
{
	World w = makeWalledWorld();
	// Tile floor is row y=4, so top of floor is at y = 4*TILE = 128.
	TestEnemy e({5.f * TILE, 4.f * TILE}, 28.f, 28.f);
	REQUIRE(EnemyTestAccess::isGroundBelow(e, w));
}

TEST_CASE("isGroundBelow reports false when enemy is floating mid-air")
{
	World w = makeEmptyWorld();
	TestEnemy e({5.f * TILE, 2.f * TILE}, 28.f, 28.f);
	REQUIRE_FALSE(EnemyTestAccess::isGroundBelow(e, w));
}

TEST_CASE("applyGravity accelerates downward when not grounded")
{
	World w = makeEmptyWorld();
	TestEnemy e({5.f * TILE, 2.f * TILE}, 28.f, 28.f);
	EnemyTestAccess::setOnGround(e, false);

	float vyAfter = EnemyTestAccess::applyGravity(e, 0.1f, w);

	// GRAVITY=1200, dt=0.1 → vy should be 120
	REQUIRE(vyAfter == 120.f);
	REQUIRE_FALSE(e.isOnGroundFlag());
}

TEST_CASE("applyGravity does not accelerate when ground is below")
{
	World w = makeWalledWorld();
	TestEnemy e({5.f * TILE, 4.f * TILE}, 28.f, 28.f);
	EnemyTestAccess::setOnGround(e, true);

	float vyAfter = EnemyTestAccess::applyGravity(e, 0.1f, w);
	REQUIRE(vyAfter == 0.f);
}

TEST_CASE("resolveHorizontal moves freely through empty space")
{
	World w = makeEmptyWorld();
	TestEnemy e({5.f * TILE, 2.f * TILE}, 28.f, 28.f);
	EnemyTestAccess::setVel(e, {120.f, 0.f});

	float x = EnemyTestAccess::resolveHorizontal(e, 0.1f, w);
	REQUIRE(x == 5.f * TILE + 12.f); // 120 * 0.1 = 12
}

TEST_CASE("resolveHorizontal stops at wall and zeros horizontal velocity")
{
	World w = makeWalledWorld();
	// Place enemy so one step at moderate speed overlaps the right wall tile.
	// Wall tile column 9 spans x=[288, 320]; enemy right edge starts ~5px from it.
	TestEnemy e({9.f * TILE - 14.f - 5.f, 2.f * TILE}, 28.f, 28.f);
	EnemyTestAccess::setVel(e, {200.f, 0.f});

	float x = EnemyTestAccess::resolveHorizontal(e, 0.1f, w);

	// Formula: adjacent-tile.x + TILE - width/2 - 1.
	// Current tile (8, 2) has x=256 → expected = 256 + 32 - 14 - 1 = 273.
	REQUIRE(x == 273.f);
	REQUIRE(e.getVelocity().x == 0.f);
}

TEST_CASE("resolveVertical plants enemy on floor and sets isOnGround")
{
	World w = makeWalledWorld();
	// Start 16 px above the floor. With vel=300 and dt=0.1, enemy falls 30 px —
	// enough for the destination rect to overlap the floor tile without the head
	// tunneling past the floor's top in a single step.
	TestEnemy e({5.f * TILE, 3.5f * TILE}, 28.f, 28.f);
	EnemyTestAccess::setVel(e, {0.f, 300.f});

	float y = EnemyTestAccess::resolveVertical(e, 0.1f, w);

	// Foot position should snap to top of floor (y = 4*TILE).
	REQUIRE(y == 4.f * TILE);
	REQUIRE(e.isOnGroundFlag());
	REQUIRE(e.getVelocity().y == 0.f);
}

TEST_CASE("resolveVertical bumps head on ceiling and zeros velocity")
{
	World w = makeWalledWorld();
	// Head just below ceiling; rise at a speed slow enough that the destination
	// rect still overlaps the ceiling tile rather than tunneling over it.
	TestEnemy e({5.f * TILE, 2.f * TILE}, 28.f, 28.f);
	EnemyTestAccess::setVel(e, {0.f, -200.f});

	float y = EnemyTestAccess::resolveVertical(e, 0.1f, w);

	// Ceiling tile spans y=[0,32]. Formula: tile.y + TILE + height = 0 + 32 + 28 = 60.
	REQUIRE(y == 60.f);
	REQUIRE(e.getVelocity().y == 0.f);
}

TEST_CASE("BaseEnemy::update runs the current state and its animation")
{
	World w = makeEmptyWorld();
	TestEnemy e({5.f * TILE, 2.f * TILE}, 28.f, 28.f);
	TestState s;
	EnemyTestAccess::setCurrentState(e, &s);

	e.update(0.016f, w, {0.f, 0.f});

	REQUIRE(s.updateCalls == 1);
	REQUIRE(s.animationCalls == 1);
}

TEST_CASE("BaseEnemy::update calls onExit/onEnter on state transition")
{
	World w = makeEmptyWorld();
	TestEnemy e({5.f * TILE, 2.f * TILE}, 28.f, 28.f);
	TestState from, to;
	from.nextState = &to;
	EnemyTestAccess::setCurrentState(e, &from);

	e.update(0.016f, w, {0.f, 0.f});

	REQUIRE(from.exitCalls == 1);
	REQUIRE(to.enterCalls == 1);
	REQUIRE(EnemyTestAccess::getCurrentState(e) == &to);
}

TEST_CASE("BaseEnemy::update runs updateAnimation on the *new* state after transition")
{
	// Contract: animation belongs to the currently-active state at the end of the tick.
	World w = makeEmptyWorld();
	TestEnemy e({5.f * TILE, 2.f * TILE}, 28.f, 28.f);
	TestState from, to;
	from.nextState = &to;
	EnemyTestAccess::setCurrentState(e, &from);

	e.update(0.016f, w, {0.f, 0.f});

	REQUIRE(from.animationCalls == 0);
	REQUIRE(to.animationCalls == 1);
}

TEST_CASE("BaseEnemy::update updates facing from player position")
{
	World w = makeEmptyWorld();
	TestEnemy e({5.f * TILE, 2.f * TILE}, 28.f, 28.f);
	TestState s;
	EnemyTestAccess::setCurrentState(e, &s);

	SECTION("player to the right → face right")
	{
		EnemyTestAccess::setFacing(e, BaseEnemy::Direction::Left);
		e.update(0.016f, w, {e.getPosition().x + 100.f, e.getPosition().y});
		REQUIRE(e.getFacing() == BaseEnemy::Direction::Right);
	}

	SECTION("player to the left → face left")
	{
		EnemyTestAccess::setFacing(e, BaseEnemy::Direction::Right);
		e.update(0.016f, w, {e.getPosition().x - 100.f, e.getPosition().y});
		REQUIRE(e.getFacing() == BaseEnemy::Direction::Left);
	}
}

TEST_CASE("BaseEnemy::update invokes onPreUpdate once per tick with dt")
{
	World w = makeEmptyWorld();
	TestEnemy e({5.f * TILE, 2.f * TILE}, 28.f, 28.f);
	TestState s;
	EnemyTestAccess::setCurrentState(e, &s);

	e.update(0.033f, w, {0.f, 0.f});

	REQUIRE(e.preUpdateCalls == 1);
	REQUIRE(e.lastPreUpdateDt == 0.033f);
}
