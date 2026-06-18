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

TEST_CASE("SegfaultBoss enters an invincible transition when it reaches stage two")
{
	World w = makeWorld();
	SegfaultBoss boss(bossSpawn());

	boss.takeDamage(SegfaultBoss::BOSS_HEALTH - SegfaultBoss::STAGE2_HP);
	REQUIRE(boss.health.current == SegfaultBoss::STAGE2_HP);

	boss.update(0.016f, w, farPlayer(boss), {});
	REQUIRE(boss.getState() == &boss.states.stage2Transition);
	REQUIRE(boss.isInvulnerable());
	REQUIRE(boss.getStage() == 1); // stage advances only once the summon fires
}

TEST_CASE("SegfaultBoss summons a wave of processes on entering stage two")
{
	World w = makeWorld();
	SegfaultBoss boss(bossSpawn());

	boss.takeDamage(SegfaultBoss::BOSS_HEALTH - SegfaultBoss::STAGE2_HP);
	boss.update(0.016f, w, farPlayer(boss), {}); // -> stage-two transition

	bool sawSummon = false;
	for (int frame = 0; frame < 200 && !sawSummon; ++frame) {
		boss.update(0.05f, w, farPlayer(boss), {});
		if (boss.summonedProcessCount() > 0)
			sawSummon = true;
	}

	REQUIRE(sawSummon);
	REQUIRE(boss.summonedProcessCount() == static_cast<std::size_t>(SegfaultBoss::SUMMON_COUNT));
	REQUIRE(boss.isStage2Triggered());
	REQUIRE(boss.getStage() == 2);

	// The boss exposes the summons' hurtboxes so the player can destroy them.
	std::vector<Hurtbox> hurtboxes;
	boss.collectHurtboxes(hurtboxes);
	REQUIRE(hurtboxes.size() > 1u); // the boss body plus each summon
}

TEST_CASE("SegfaultBoss only triggers the stage-two summon once")
{
	World w = makeWorld();
	SegfaultBoss boss(bossSpawn());

	boss.takeDamage(SegfaultBoss::BOSS_HEALTH - SegfaultBoss::STAGE2_HP);

	// Run through the transition + summon back to roaming.
	bool resumedRoaming = false;
	for (int frame = 0; frame < 400 && !resumedRoaming; ++frame) {
		boss.update(0.05f, w, farPlayer(boss), {});
		if (boss.isStage2Triggered() && boss.getState() == &boss.states.roaming)
			resumedRoaming = true;
	}
	REQUIRE(resumedRoaming);

	// Already in stage two: roaming must not re-enter the transition.
	for (int frame = 0; frame < 20; ++frame) {
		boss.update(0.05f, w, farPlayer(boss), {});
		REQUIRE(boss.getState() != &boss.states.stage2Transition);
	}
}

TEST_CASE("SegfaultBoss leaks corruption blocks that deal contact damage")
{
	World w = makeWorld();
	SegfaultBoss boss(bossSpawn());
	const sf::Vector2f player = boss.getPosition() + sf::Vector2f{60.f, 0.f};

	// While a block is present it publishes a hazard hitbox at the stage-one damage.
	bool sawHazard = false;
	for (int frame = 0; frame < 600 && !sawHazard; ++frame) {
		boss.update(0.05f, w, player, {});
		if (boss.corruptionBlockCount() == 0)
			continue;
		std::vector<Hitbox> hitboxes;
		boss.collectHitboxes(hitboxes);
		for (const Hitbox &hitbox : hitboxes)
			if (hitbox.damage == SegfaultBoss::CORRUPTION_DAMAGE_BASE)
				sawHazard = true;
	}
	REQUIRE(sawHazard);
}

TEST_CASE("SegfaultBoss corruption blocks expire after their lifetime")
{
	World w = makeWorld();
	SegfaultBoss boss(bossSpawn());
	const sf::Vector2f player = boss.getPosition() + sf::Vector2f{60.f, 0.f};

	for (int frame = 0; frame < 600 && boss.corruptionBlockCount() == 0; ++frame)
		boss.update(0.05f, w, player, {});
	REQUIRE(boss.corruptionBlockCount() > 0);

	const float age = boss.getCorruptionBlocks().front().age;
	// Run past the remaining lifetime of the oldest block; it must be cleared.
	const int frames = static_cast<int>((SegfaultBoss::CORRUPTION_LIFETIME - age) / 0.05f) + 4;
	const std::size_t before = boss.corruptionBlockCount();
	bool sawExpiry = false;
	for (int frame = 0; frame < frames && !sawExpiry; ++frame) {
		boss.update(0.05f, w, player, {});
		if (boss.corruptionBlockCount() < before)
			sawExpiry = true;
	}
	REQUIRE(sawExpiry);
}

TEST_CASE("SegfaultBoss corruption grows more damaging in later stages")
{
	World w = makeWorld();
	SegfaultBoss boss(bossSpawn());
	const sf::Vector2f player = boss.getPosition() + sf::Vector2f{60.f, 0.f};

	boss.takeDamage(SegfaultBoss::BOSS_HEALTH - SegfaultBoss::STAGE2_HP);

	bool sawEscalatedBlock = false;
	for (int frame = 0; frame < 1200 && !sawEscalatedBlock; ++frame) {
		boss.update(0.05f, w, player, {});
		for (const SegfaultBoss::CorruptionBlock &block : boss.getCorruptionBlocks())
			if (block.damage > SegfaultBoss::CORRUPTION_DAMAGE_BASE)
				sawEscalatedBlock = true;
	}
	REQUIRE(sawEscalatedBlock);
	REQUIRE(boss.getStage() == 2);
}

TEST_CASE("SegfaultBoss clears corruption blocks when defeated")
{
	World w = makeWorld();
	SegfaultBoss boss(bossSpawn());
	const sf::Vector2f player = boss.getPosition() + sf::Vector2f{60.f, 0.f};

	for (int frame = 0; frame < 600 && boss.corruptionBlockCount() == 0; ++frame)
		boss.update(0.05f, w, player, {});
	REQUIRE(boss.corruptionBlockCount() > 0);

	boss.takeDamage(SegfaultBoss::BOSS_HEALTH);
	boss.update(0.016f, w, player, {});

	REQUIRE(boss.getState() == &boss.states.death);
	REQUIRE(boss.corruptionBlockCount() == 0u);
}

TEST_CASE("SegfaultBoss clears its summons when defeated")
{
	World w = makeWorld();
	SegfaultBoss boss(bossSpawn());

	boss.takeDamage(SegfaultBoss::BOSS_HEALTH - SegfaultBoss::STAGE2_HP);
	for (int frame = 0; frame < 200 && boss.summonedProcessCount() == 0; ++frame)
		boss.update(0.05f, w, farPlayer(boss), {});
	REQUIRE(boss.summonedProcessCount() > 0);

	boss.takeDamage(SegfaultBoss::BOSS_HEALTH);
	boss.update(0.016f, w, farPlayer(boss), {});

	REQUIRE(boss.getState() == &boss.states.death);
	REQUIRE(boss.summonedProcessCount() == 0u);
}
