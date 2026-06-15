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

TEST_CASE("SegfaultBoss telegraphs the first NULL spear on the player, then strikes")
{
	World w = makeWorld();
	SegfaultBoss boss(bossSpawn());
	const sf::Vector2f player = boss.getPosition() + sf::Vector2f{40.f, 0.f};

	boss.update(0.016f, w, player, {});
	REQUIRE(boss.getState() == &boss.states.nullSpearAttack);
	REQUIRE(boss.getSpears().size() == 1u);
	REQUIRE(boss.getSpears().front().phase == SegfaultBoss::SpearPhase::Windup);
	REQUIRE(boss.getSpears().front().foot.x == player.x);
	REQUIRE(boss.getSpears().front().foot.y == 20.f * TILE); // on top of the floor row

	// While telegraphed, no damage is dealt.
	std::vector<Hitbox> hitboxes;
	boss.collectHitboxes(hitboxes);
	REQUIRE(hitboxes.empty());

	// Once a spear finishes its windup it strikes and publishes a damaging hitbox.
	bool sawStrike = false;
	for (int frame = 0; frame < 100 && !sawStrike; ++frame) {
		boss.update(0.05f, w, player, {});
		hitboxes.clear();
		boss.collectHitboxes(hitboxes);
		if (!hitboxes.empty()) {
			sawStrike = true;
			REQUIRE(hitboxes.front().damage == SegfaultBoss::SPEAR_DAMAGE);
		}
	}
	REQUIRE(sawStrike);
}

TEST_CASE("SegfaultBoss telegraphs several NULL spears staggered in time")
{
	World w = makeWorld();
	SegfaultBoss boss(bossSpawn());
	const sf::Vector2f player = boss.getPosition() + sf::Vector2f{40.f, 0.f};

	boss.update(0.016f, w, player, {}); // enters attack, first spear appears
	REQUIRE(boss.getSpears().size() == 1u);

	// A second spear is not telegraphed until roughly the stagger interval has passed.
	boss.update(SegfaultBoss::SPEAR_SPAWN_INTERVAL * 0.5f, w, player, {});
	const std::size_t afterHalfInterval = boss.getSpears().size();
	boss.update(SegfaultBoss::SPEAR_SPAWN_INTERVAL, w, player, {});
	REQUIRE(boss.getSpears().size() > afterHalfInterval);

	bool sawRecover = false;
	for (int frame = 0; frame < 400 && !sawRecover; ++frame) {
		boss.update(0.05f, w, player, {});
		if (boss.getState() == &boss.states.recover)
			sawRecover = true;
	}
	REQUIRE(sawRecover);
}

TEST_CASE("SegfaultBoss aims some NULL spears at the player's live position")
{
	World w = makeWorld();
	SegfaultBoss boss(bossSpawn());
	sf::Vector2f player = boss.getPosition() + sf::Vector2f{40.f, 0.f};

	boss.update(0.016f, w, player, {});
	REQUIRE(boss.getSpears().size() == 1u);
	REQUIRE(boss.getSpears().front().foot.x == player.x);

	player.x += 100.f;
	boss.update(SegfaultBoss::SPEAR_SPAWN_INTERVAL + 0.01f, w, player, {});
	REQUIRE(boss.getSpears().size() == 2u);
	REQUIRE(boss.getSpears().back().foot.x == player.x);
}

TEST_CASE("SegfaultBoss drops NULL spears onto the floor below an airborne player")
{
	World w = makeWorld();
	SegfaultBoss boss(bossSpawn());

	const sf::Vector2f airPlayer{boss.getPosition().x + 30.f, 12.f * TILE};

	boss.update(0.016f, w, airPlayer, {});
	REQUIRE(boss.getState() == &boss.states.nullSpearAttack);
	REQUIRE(boss.getSpears().size() == 1u);
	REQUIRE(boss.getSpears().front().foot.x == airPlayer.x);
	REQUIRE(boss.getSpears().front().foot.y == 20.f * TILE);
}

TEST_CASE("SegfaultBoss does not attack while the player is out of spear range")
{
	World w = makeWorld();
	SegfaultBoss boss(bossSpawn());

	boss.update(0.016f, w, farPlayer(boss), {});
	REQUIRE(boss.getState() == &boss.states.roaming);
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
