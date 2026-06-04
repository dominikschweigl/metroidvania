#include <catch2/catch_test_macros.hpp>

#include "core/audio_manager.h"
#include "entities/enemies/bosses/transistor_boss/transistor_boss.h"
#include "entities/enemies/capacitor/capacitor.h"
#include "world/world.h"

#include <algorithm>
#include <vector>

namespace {

constexpr float TILE = World::TILE_SIZE;

World makeWorld()
{
	std::vector<std::vector<int>> g(24, std::vector<int>(60, 0));
	for (int x = 0; x < 60; ++x)
		g[20][x] = 1; // floor
	World w;
	w.loadFromGrid(g);
	return w;
}

sf::Vector2f bossSpawn()
{
	return {30.f * TILE, 20.f * TILE};
}

// Player far enough away that the boss won't peel off into a shoot/charge attack.
sf::Vector2f farPlayer(const TransistorBoss &boss)
{
	return boss.getPosition() + sf::Vector2f{2000.f, 0.f};
}

bool hasBeamHitbox(const std::vector<Hitbox> &hitboxes)
{
	// Beam boxes are the fixed-size squares emitted along each tether.
	return std::any_of(hitboxes.begin(), hitboxes.end(), [](const Hitbox &h) {
		return h.damage == TransistorBoss::BEAM_DAMAGE && h.bounds.size.x == TransistorBoss::BEAM_HITBOX_SIZE;
	});
}

} // namespace

TEST_CASE("TransistorBoss starts roaming in stage one")
{
	TransistorBoss boss(bossSpawn());
	REQUIRE(boss.getState() == &boss.states.roaming);
	REQUIRE_FALSE(boss.isStage2Triggered());
	REQUIRE(boss.bondedCapacitorCount() == 0u);
}

TEST_CASE("TransistorBoss enters the second stage once it drops to half health")
{
	World w = makeWorld();

	SECTION("above the threshold it keeps roaming")
	{
		TransistorBoss boss(bossSpawn());
		boss.takeDamage(TransistorBoss::BOSS_HEALTH - TransistorBoss::STAGE2_HP - 1); // one HP above
		REQUIRE(boss.states.roaming.update(0.016f, boss, w, farPlayer(boss)) == &boss.states.roaming);
	}

	SECTION("at the threshold it transitions to the stage-two recover")
	{
		TransistorBoss boss(bossSpawn());
		boss.takeDamage(TransistorBoss::BOSS_HEALTH - TransistorBoss::STAGE2_HP); // current == STAGE2_HP
		REQUIRE(boss.states.roaming.update(0.016f, boss, w, farPlayer(boss)) == &boss.states.stage2Recover);
	}
}

TEST_CASE("TransistorBoss is invincible through the longer stage-two recover, then summons")
{
	World w = makeWorld();
	TransistorBoss boss(bossSpawn());

	REQUIRE_FALSE(boss.isInvulnerable());

	boss.states.stage2Recover.onEnter(boss);
	REQUIRE(boss.isInvulnerable());
	REQUIRE(boss.getHurtbox().invulnerable);

	// Holds through the (longer) recover, then advances to summon.
	REQUIRE(boss.states.stage2Recover.update(0.5f, boss, w, farPlayer(boss)) == &boss.states.stage2Recover);
	REQUIRE(boss.states.stage2Recover.update(TransistorBoss::STAGE2_RECOVER_DUR, boss, w, farPlayer(boss))
	        == &boss.states.summon);

	// No minions appear until the summon itself runs.
	REQUIRE(boss.bondedCapacitorCount() == 0u);

	// Leaving the recover restores vulnerability for the actual stage-two fight.
	boss.states.stage2Recover.onExit(boss);
	REQUIRE_FALSE(boss.isInvulnerable());
}

TEST_CASE("TransistorBoss summon spawns the bonded capacitors then resumes roaming")
{
	World w = makeWorld();
	TransistorBoss boss(bossSpawn());

	boss.states.summon.onEnter(boss);
	REQUIRE(boss.isStage2Triggered());
	REQUIRE(boss.bondedCapacitorCount() == static_cast<std::size_t>(TransistorBoss::CAPACITOR_COUNT));

	const sf::Vector2f player = boss.getPosition() + sf::Vector2f{120.f, 0.f};
	REQUIRE(boss.states.summon.update(0.1f, boss, w, player) == &boss.states.summon);
	REQUIRE(boss.states.summon.update(TransistorBoss::SUMMON_DUR, boss, w, player) == &boss.states.roaming);
}

TEST_CASE("TransistorBoss tethers damaging beams only while bonded capacitors live")
{
	World w = makeWorld();
	TransistorBoss boss(bossSpawn());
	boss.takeDamage(TransistorBoss::BOSS_HEALTH - TransistorBoss::STAGE2_HP);

	const sf::Vector2f player = boss.getPosition() + sf::Vector2f{300.f, 0.f};

	// Frame 1: roaming -> stage-two recover (invincible pause, no minions yet).
	boss.update(0.016f, w, player, {});
	REQUIRE(boss.bondedCapacitorCount() == 0u);

	// Recover elapses -> summon spawns the capacitors.
	boss.update(TransistorBoss::STAGE2_RECOVER_DUR + 0.1f, w, player, {});
	REQUIRE(boss.bondedCapacitorCount() == static_cast<std::size_t>(TransistorBoss::CAPACITOR_COUNT));

	// Next frame the beam id is armed, so beam hitboxes are published.
	boss.update(0.016f, w, player, {});
	std::vector<Hitbox> hitboxes;
	boss.collectHitboxes(hitboxes);
	REQUIRE(hasBeamHitbox(hitboxes));

	// Destroy every bonded capacitor through its published hurtbox.
	std::vector<Hurtbox> hurtboxes;
	boss.collectHurtboxes(hurtboxes);
	for (Hurtbox &hurt : hurtboxes)
		if (hurt.health != &boss.health)
			hurt.health->damage(Capacitor::CAPACITOR_HEALTH);

	// Next frame the boss prunes them and the beams vanish.
	boss.update(0.016f, w, player, {});
	REQUIRE(boss.bondedCapacitorCount() == 0u);

	hitboxes.clear();
	boss.collectHitboxes(hitboxes);
	REQUIRE_FALSE(hasBeamHitbox(hitboxes));
}

TEST_CASE("TransistorBoss summons the second stage only once")
{
	World w = makeWorld();
	TransistorBoss boss(bossSpawn());
	boss.takeDamage(TransistorBoss::BOSS_HEALTH - TransistorBoss::STAGE2_HP); // sits at the threshold

	// Trigger the stage and spawn the capacitors.
	boss.states.summon.onEnter(boss);
	REQUIRE(boss.isStage2Triggered());

	// Destroy every bonded capacitor, then let the boss prune them.
	std::vector<Hurtbox> hurtboxes;
	boss.collectHurtboxes(hurtboxes);
	for (Hurtbox &hurt : hurtboxes)
		if (hurt.health != &boss.health)
			hurt.health->damage(Capacitor::CAPACITOR_HEALTH);
	boss.update(0.016f, w, farPlayer(boss), {});
	REQUIRE(boss.bondedCapacitorCount() == 0u);

	// Still at half health, but the second stage must not re-trigger.
	REQUIRE(boss.states.roaming.update(0.016f, boss, w, farPlayer(boss)) == &boss.states.roaming);
}

TEST_CASE("TransistorBoss enters its death state and is never removed once defeated")
{
	World w = makeWorld();
	TransistorBoss boss(bossSpawn());

	// Summon the stage-two minions so we can confirm they are cleared on death.
	boss.states.summon.onEnter(boss);
	REQUIRE(boss.bondedCapacitorCount() == static_cast<std::size_t>(TransistorBoss::CAPACITOR_COUNT));

	boss.takeDamage(TransistorBoss::BOSS_HEALTH);
	REQUIRE_FALSE(boss.health.isAlive());

	// The next frame the boss detects the defeat and switches to the death state.
	boss.update(0.016f, w, farPlayer(boss), {});

	REQUIRE(boss.getState() == &boss.states.death);
	REQUIRE(boss.isAlive());        // stays in the scene to show its death animation
	REQUIRE(boss.isInvulnerable()); // can no longer be hit
	REQUIRE(boss.bondedCapacitorCount() == 0u);

	for (int frame = 0; frame < 80; ++frame)
		boss.update(0.2f, w, farPlayer(boss), {});

	REQUIRE(boss.getState() == &boss.states.death);
	REQUIRE(boss.isAlive());
	REQUIRE(AudioManager::getInstance().musicStatus() == MusicStatus::Playing);
}

TEST_CASE("TransistorBoss exposes its bonded capacitors as destroyable targets")
{
	TransistorBoss boss(bossSpawn());
	boss.states.summon.onEnter(boss); // spawns the capacitors

	std::vector<Hurtbox> hurtboxes;
	boss.collectHurtboxes(hurtboxes);

	// The boss body plus one hurtbox per bonded capacitor.
	REQUIRE(hurtboxes.size() == static_cast<std::size_t>(TransistorBoss::CAPACITOR_COUNT) + 1);
	const std::size_t enemyTargets =
	    static_cast<std::size_t>(std::count_if(hurtboxes.begin(), hurtboxes.end(), [&boss](const Hurtbox &h) {
		    return h.team == Team::Enemy && h.health != &boss.health;
	    }));
	REQUIRE(enemyTargets == static_cast<std::size_t>(TransistorBoss::CAPACITOR_COUNT));
}
