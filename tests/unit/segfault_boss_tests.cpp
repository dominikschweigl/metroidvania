#include <catch2/catch_test_macros.hpp>

#include "entities/enemies/bosses/segfault_boss/segfault_boss.h"
#include "world/world.h"

#include <vector>

namespace {

constexpr float TILE = World::TILE_SIZE;

World makeWorld()
{
	std::vector<std::vector<int>> g(24, std::vector<int>(60, 0));
	for (int x = 0; x < 60; ++x)
		g[20][x] = 1; // floor
	World w = World("test");
	w.loadFromGrid(g);
	return w;
}

sf::Vector2f bossSpawn()
{
	return {30.f * TILE, 20.f * TILE};
}

// Player far enough away that the boss patrols.
sf::Vector2f farPlayer(const SegfaultBoss &boss)
{
	return boss.getPosition() + sf::Vector2f{2000.f, 0.f};
}

} // namespace

TEST_CASE("SegfaultBoss starts roaming at full health")
{
	SegfaultBoss boss(bossSpawn());
	REQUIRE(boss.getState() == &boss.states.roaming);
	REQUIRE(boss.health.current == SegfaultBoss::BOSS_HEALTH);
	REQUIRE(boss.isAlive());
	REQUIRE_FALSE(boss.isInvulnerable());
}

TEST_CASE("SegfaultBoss patrols along the ground")
{
	World w = makeWorld();
	SegfaultBoss boss(bossSpawn());

	boss.update(0.016f, w, farPlayer(boss), {});
	// Roaming drives a non-zero horizontal velocity.
	REQUIRE(boss.getVelocity().x != 0.f);
}

TEST_CASE("SegfaultBoss enters its death state and is never removed once defeated")
{
	World w = makeWorld();
	SegfaultBoss boss(bossSpawn());

	boss.takeDamage(SegfaultBoss::BOSS_HEALTH);
	REQUIRE_FALSE(boss.health.isAlive());

	// Next frame the boss detects the defeat and switches to the death state.
	boss.update(0.016f, w, farPlayer(boss), {});

	REQUIRE(boss.getState() == &boss.states.death);
	REQUIRE(boss.isAlive());
	REQUIRE(boss.isInvulnerable());

	for (int frame = 0; frame < 40; ++frame)
		boss.update(0.1f, w, farPlayer(boss), {});

	REQUIRE(boss.getState() == &boss.states.death);
	REQUIRE(boss.isAlive());
}

TEST_CASE("SegfaultBoss does not request a bluescreen during stage one")
{
	SegfaultBoss boss(bossSpawn());
	REQUIRE_FALSE(boss.consumeBluescreenRequest());
}
