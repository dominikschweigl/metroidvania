#include <catch2/catch_test_macros.hpp>

#include "../../src/combat/combat_system.h"
#include "../../src/entities/base/base_enemy.h"
#include "../../src/entities/base/enemy_state.h"
#include "../../src/entities/player/hat_projectile.h"
#include "../../src/world/world.h"
#include <SFML/Graphics.hpp>
#include <vector>

namespace {

World makeEmptyWorld()
{
	std::vector<std::vector<int>> grid(5, std::vector<int>(10, 0));
	World world;
	world.loadFromGrid(grid);
	return world;
}

World makeWalledWorld()
{
	std::vector<std::vector<int>> grid(5, std::vector<int>(10, 0));
	for (int x = 0; x < 10; ++x) {
		grid[0][x] = 1;
		grid[4][x] = 1;
	}
	for (int y = 0; y < 5; ++y) {
		grid[y][0] = 1;
		grid[y][9] = 1;
	}
	World world;
	world.loadFromGrid(grid);
	return world;
}

class TestEnemy : public BaseEnemy {
  public:
	TestEnemy(sf::Vector2f spawn, float w = 32.f, float h = 32.f) : BaseEnemy(spawn, w, h) {}
	void draw(sf::RenderWindow &) override {}
};

class NullState : public EnemyState {
  public:
	EnemyState *update(float, BaseEnemy &, const World &, sf::Vector2f) override { return this; }
	void updateAnimation(float, BaseEnemy &) override {}
	void onEnter(BaseEnemy &) override {}
	void onExit(BaseEnemy &) override {}
};

constexpr float TILE = World::TILE_SIZE;

} // namespace

TEST_CASE("HatProjectile starts in Flying phase and moves in throw direction")
{
	sf::Texture dummyTexture;
	World world = makeEmptyWorld();

	HatProjectile hat({5.f * TILE, 2.f * TILE}, Direction::Right, {0.f, 0.f}, dummyTexture);

	const float initialX = hat.getBounds().position.x;
	hat.update(0.1f, {5.f * TILE, 2.f * TILE}, world);

	REQUIRE(hat.getBounds().position.x > initialX);
}

TEST_CASE("HatProjectile thrown left moves in negative X direction")
{
	sf::Texture dummyTexture;
	World world = makeEmptyWorld();

	HatProjectile hat({5.f * TILE, 2.f * TILE}, Direction::Left, {0.f, 0.f}, dummyTexture);

	const float initialX = hat.getBounds().position.x;
	hat.update(0.1f, {5.f * TILE, 2.f * TILE}, world);

	REQUIRE(hat.getBounds().position.x < initialX);
}

TEST_CASE("HatProjectile switches to Returning after MAX_TRAVEL")
{
	sf::Texture dummyTexture;
	World world = makeEmptyWorld();
	const sf::Vector2f playerPos{5.f * TILE, 2.f * TILE};

	HatProjectile hat(playerPos, Direction::Right, {0.f, 0.f}, dummyTexture);

	// Advance past MAX_TRAVEL; hat travels HAT_SPEED px/s rightward.
	// MAX_TRAVEL / HAT_SPEED seconds is the minimum time to cross the threshold.
	const float travelTime = HatProjectile::BASE_TRAVEL / HatProjectile::HAT_SPEED + 0.01f;
	bool caught = hat.update(travelTime, playerPos, world);

	// Not yet caught (hat is now returning but far from player); should not be caught immediately.
	REQUIRE_FALSE(caught);

	// Verify it's homing back: give it enough time to return and catch up.
	// Once returning it will home toward playerPos, which is at the start.
	const float returnTime = (HatProjectile::BASE_TRAVEL / HatProjectile::RETURN_SPEED) + 0.01f;
	caught = hat.update(returnTime, playerPos, world);

	REQUIRE(caught);
}

TEST_CASE("HatProjectile reverses at wall before MAX_TRAVEL")
{
	sf::Texture dummyTexture;
	World world = makeWalledWorld();
	// Start near the right wall; wall is at column 9 (x=288). Hat is 32px wide.
	const sf::Vector2f startPos{8.f * TILE, 2.f * TILE};
	const sf::Vector2f playerPos = startPos;

	HatProjectile hat(startPos, Direction::Right, {0.f, 0.f}, dummyTexture);

	// One small tick: should not have travelled MAX_TRAVEL yet, but hits the wall.
	hat.update(0.05f, playerPos, world);

	// Now provide enough time for the hat to return to the player.
	const bool caught = hat.update(1.0f, playerPos, world);
	REQUIRE(caught);
}

TEST_CASE("HatProjectile::getHitbox carries Player team, positive damage, and stable sourceId")
{
	sf::Texture dummyTexture;
	const sf::Vector2f pos{5.f * TILE, 2.f * TILE};
	HatProjectile hat(pos, Direction::Right, {0.f, 0.f}, dummyTexture);

	const Hitbox first = hat.getHitbox();
	const Hitbox second = hat.getHitbox();

	REQUIRE(first.team == Team::Player);
	REQUIRE(first.damage > 0);
	REQUIRE(first.sourceId != 0u);
	REQUIRE(first.sourceId == second.sourceId);
	REQUIRE(first.bounds.position == hat.getBounds().position);
}

TEST_CASE("Distinct HatProjectile instances get distinct source IDs")
{
	sf::Texture dummyTexture;
	HatProjectile a({0.f, 0.f}, Direction::Right, {0.f, 0.f}, dummyTexture);
	HatProjectile b({0.f, 0.f}, Direction::Right, {0.f, 0.f}, dummyTexture);
	REQUIRE(a.getHitbox().sourceId != b.getHitbox().sourceId);
}

TEST_CASE("HatProjectile + CombatSystem: damages overlapping enemy once per throw")
{
	sf::Texture dummyTexture;
	const sf::Vector2f pos{5.f * TILE, 2.f * TILE};
	HatProjectile hat(pos, Direction::Right, {0.f, 0.f}, dummyTexture);

	NullState state;
	TestEnemy enemy(pos);
	enemy.setState(&state);

	const int hpBefore = enemy.health.current;
	CombatSystem combat;
	std::vector<Hitbox> hits{hat.getHitbox()};
	std::vector<Hurtbox> hurts{enemy.getHurtbox()};

	combat.resolve(hits, hurts);
	combat.resolve(hits, hurts);
	combat.resolve(hits, hurts);

	REQUIRE(enemy.health.current == hpBefore - HatProjectile::DAMAGE);
}

TEST_CASE("HatProjectile + CombatSystem: non-overlapping enemy takes no damage")
{
	sf::Texture dummyTexture;
	HatProjectile hat({5.f * TILE, 2.f * TILE}, Direction::Right, {0.f, 0.f}, dummyTexture);

	NullState state;
	TestEnemy enemy({0.f, 0.f});
	enemy.setState(&state);

	const int hpBefore = enemy.health.current;
	CombatSystem combat;
	std::vector<Hitbox> hits{hat.getHitbox()};
	std::vector<Hurtbox> hurts{enemy.getHurtbox()};
	combat.resolve(hits, hurts);

	REQUIRE(enemy.health.current == hpBefore);
}

TEST_CASE("HatProjectile is caught when returned within CATCH_RADIUS")
{
	sf::Texture dummyTexture;
	World world = makeEmptyWorld();
	const sf::Vector2f playerPos{5.f * TILE, 2.f * TILE};

	HatProjectile hat(playerPos, Direction::Right, {0.f, 0.f}, dummyTexture);

	// Advance past MAX_TRAVEL to switch to Returning phase.
	const float travelTime = HatProjectile::BASE_TRAVEL / HatProjectile::HAT_SPEED + 0.01f;
	hat.update(travelTime, playerPos, world);

	// Player has moved to the hat's current position — distance is zero, within CATCH_RADIUS.
	const sf::FloatRect hatBounds = hat.getBounds();
	const sf::Vector2f hatCenter{hatBounds.position.x + hatBounds.size.x / 2.f,
	                             hatBounds.position.y + hatBounds.size.y / 2.f};
	const bool caught = hat.update(0.01f, hatCenter, world);

	REQUIRE(caught);
}
