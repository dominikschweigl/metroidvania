#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "combat/hitbox.h"
#include "effects/effect.h"
#include "entities/enemies/recursion_golem/recursion_golem.h"
#include "entities/enemies/recursion_golem/states/attack_state.h"
#include "entities/enemies/recursion_golem/states/chase_state.h"
#include "entities/enemies/recursion_golem/states/explode_state.h"
#include "entities/enemies/recursion_golem/states/idle_state.h"
#include "entities/player/player.h"
#include "world/world.h"
#include "world_test_helpers.h"

#include <cmath>
#include <memory>
#include <vector>

namespace {

constexpr float TILE = World::TILE_SIZE;

World makeOpenWorld()
{
	std::vector<std::vector<int>> g(20, std::vector<int>(40, 0));
	for (int x = 0; x < 40; ++x)
		g[15][x] = 1; // floor row
	World w = World("test");
	w.loadFromGrid(g);
	return w;
}

sf::Vector2f groundSpawn()
{
	return {20.f * TILE, 15.f * TILE};
}

sf::Vector2f farFromGolem(const RecursionGolem &golem)
{
	return golem.getPosition() + sf::Vector2f{RecursionGolem::LOSE_RANGE + 200.f, 0.f};
}

// Drives the golem one update with the player far away (so state stays idle and
// the defeat resolution in onPreUpdate is what we observe).
void tick(RecursionGolem &golem, World &world, float dt)
{
	golem.update(dt, world, farFromGolem(golem));
}

RecursionGolem *asGolem(const std::unique_ptr<BaseEnemy> &enemy)
{
	return static_cast<RecursionGolem *>(enemy.get());
}

} // namespace

TEST_CASE("RecursionGolem starts in Idle at its spawn position")
{
	RecursionGolem g(groundSpawn(), 3);
	REQUIRE(g.getState() == &g.states.idle);
	REQUIRE(g.getPosition() == groundSpawn());
	REQUIRE(g.getVelocity() == sf::Vector2f{0.f, 0.f});
	REQUIRE(g.getSize() == 3);
	REQUIRE(g.getAttackCooldown() == 0.f);
}

TEST_CASE("RecursionGolem geometry scales with size, speed inversely")
{
	REQUIRE(RecursionGolem::widthForSize(3) > RecursionGolem::widthForSize(1));
	REQUIRE(RecursionGolem::heightForSize(3) > RecursionGolem::heightForSize(0));

	RecursionGolem big(groundSpawn(), 3);
	RecursionGolem small(groundSpawn(), 1);
	// Smaller golems move faster.
	REQUIRE(small.moveSpeed() > big.moveSpeed());
}

TEST_CASE("RecursionGolem base-case sizes still have at least one hit point")
{
	RecursionGolem zero(groundSpawn(), 0);
	REQUIRE(zero.health.max >= 1);
	REQUIRE(zero.isAlive());
	REQUIRE(zero.isBaseCase());
}

// --- IdleState ---

TEST_CASE("Golem IdleState transitions to Chase when player enters detect range")
{
	World w = makeOpenWorld();
	RecursionGolem g(groundSpawn(), 3);
	g.states.idle.onEnter(g);

	SECTION("stays idle when player is out of range")
	{
		const sf::Vector2f far = g.getPosition() + sf::Vector2f{RecursionGolem::DETECT_RANGE + 100.f, 0.f};
		REQUIRE(g.states.idle.update(0.016f, g, w, far) == &g.states.idle);
	}

	SECTION("chases when player enters detect range")
	{
		const sf::Vector2f near = g.getPosition() + sf::Vector2f{RecursionGolem::DETECT_RANGE - 10.f, 0.f};
		REQUIRE(g.states.idle.update(0.016f, g, w, near) == &g.states.chase);
	}
}

// --- ChaseState ---

TEST_CASE("Golem ChaseState transitions")
{
	World w = makeOpenWorld();
	RecursionGolem g(groundSpawn(), 3);
	g.states.chase.onEnter(g);

	SECTION("winds up when in range and off cooldown")
	{
		g.setAttackCooldown(0.f);
		const sf::Vector2f melee = g.getPosition() + sf::Vector2f{RecursionGolem::ATTACK_RANGE - 5.f, 0.f};
		REQUIRE(g.states.chase.update(0.016f, g, w, melee) == &g.states.windup);
	}

	SECTION("waits in idle when in range but on cooldown")
	{
		g.setAttackCooldown(1.f);
		const sf::Vector2f melee = g.getPosition() + sf::Vector2f{RecursionGolem::ATTACK_RANGE - 5.f, 0.f};
		REQUIRE(g.states.chase.update(0.016f, g, w, melee) == &g.states.idle);
	}

	SECTION("gives up when player escapes beyond lose range")
	{
		const sf::Vector2f far = g.getPosition() + sf::Vector2f{RecursionGolem::LOSE_RANGE + 50.f, 0.f};
		REQUIRE(g.states.chase.update(0.016f, g, w, far) == &g.states.idle);
	}

	SECTION("moves toward the player at the size-scaled speed")
	{
		const sf::Vector2f mid = g.getPosition() + sf::Vector2f{RecursionGolem::ATTACK_RANGE + 100.f, 0.f};
		g.setDirection(Direction::Right);
		(void)g.states.chase.update(0.016f, g, w, mid);
		REQUIRE(g.getVelocity().x == Catch::Approx(g.moveSpeed()));
	}
}

// --- WindUp / AttackState ---

TEST_CASE("Golem WindUpState arms the attack cooldown and then yields to Attack")
{
	World w = makeOpenWorld();
	RecursionGolem g(groundSpawn(), 3);
	g.setAttackCooldown(0.f);

	g.states.windup.onEnter(g);
	REQUIRE(g.getAttackCooldown() == RecursionGolem::ATTACK_COOLDOWN);

	REQUIRE(g.states.windup.update(RecursionGolem::WINDUP_DUR * 0.25f, g, w, g.getPosition()) == &g.states.windup);
	REQUIRE(g.states.windup.update(RecursionGolem::WINDUP_DUR + 0.01f, g, w, g.getPosition()) == &g.states.attack);
}

TEST_CASE("Golem AttackState opens a fresh damage source on enter")
{
	RecursionGolem g(groundSpawn(), 3);
	g.states.attack.onEnter(g);
	REQUIRE(g.getAttackSourceId() != 0u);
}

TEST_CASE("Golem attack hitbox carries damage, Enemy team, and the Slow debuff")
{
	RecursionGolem g(groundSpawn(), 3);

	REQUIRE_FALSE(g.getHitbox().has_value());

	g.setState(&g.states.attack);
	g.beginAttackSource();
	const auto hit = g.getHitbox();
	REQUIRE(hit.has_value());
	REQUIRE(hit->team == Team::Enemy);
	REQUIRE(hit->damage == RecursionGolem::ATTACK_DAMAGE);
	REQUIRE(hit->statusOnHit == StatusEffectKind::Slow);
}

// --- Recursive split ---

TEST_CASE("RecursionGolem decomposes into fib children (n-1, n-2) on defeat")
{
	World w = makeOpenWorld();
	RecursionGolem g(groundSpawn(), 3);

	g.takeDamage(100);
	tick(g, w, 0.016f);

	std::vector<std::unique_ptr<BaseEnemy>> spawned;
	g.drainSpawns(spawned);

	REQUIRE(spawned.size() == 2);
	REQUIRE(asGolem(spawned[0])->getSize() == 2); // n-1
	REQUIRE(asGolem(spawned[1])->getSize() == 1); // n-2
	// Children are smaller than the parent.
	REQUIRE(RecursionGolem::widthForSize(2) < RecursionGolem::widthForSize(3));
	// The split parent is now slated for removal.
	REQUIRE_FALSE(g.isAlive());
}

TEST_CASE("A drained split parent yields its children only once")
{
	World w = makeOpenWorld();
	RecursionGolem g(groundSpawn(), 2);

	g.takeDamage(100);
	tick(g, w, 0.016f);

	std::vector<std::unique_ptr<BaseEnemy>> first;
	g.drainSpawns(first);
	REQUIRE(first.size() == 2);

	std::vector<std::unique_ptr<BaseEnemy>> second;
	g.drainSpawns(second);
	REQUIRE(second.empty());
}

// --- Stack Overflow explosion (base case) ---

TEST_CASE("Base-case RecursionGolem explodes instead of splitting")
{
	World w = makeOpenWorld();
	RecursionGolem g(groundSpawn(), 1);

	g.takeDamage(100);
	tick(g, w, 0.016f); // begins the countdown, enters Explode

	REQUIRE(g.isExploding());
	REQUIRE(g.getState() == &g.states.explode);
	REQUIRE(g.isAlive()); // survives through the countdown
	REQUIRE_FALSE(g.getHitbox().has_value());

	// It produces no children.
	std::vector<std::unique_ptr<BaseEnemy>> spawned;
	g.drainSpawns(spawned);
	REQUIRE(spawned.empty());

	// Advance past the countdown: the blast hitbox fires this frame.
	tick(g, w, RecursionGolem::EXPLODE_COUNTDOWN + 0.05f);
	const auto blast = g.getHitbox();
	REQUIRE(blast.has_value());
	REQUIRE(blast->damage == RecursionGolem::EXPLODE_DAMAGE);
	REQUIRE(blast->statusOnHit == StatusEffectKind::Slow);
	REQUIRE(blast->team == Team::Enemy);
	REQUIRE(g.isAlive()); // still alive the frame the blast lands

	// The blast area is larger than the golem's body.
	REQUIRE(blast->bounds.size.x > RecursionGolem::widthForSize(1));

	tick(g, w, RecursionGolem::EXPLOSION_FRAME_COUNT * RecursionGolem::EXPLOSION_FRAME_DURATION + 0.05f);
	REQUIRE_FALSE(g.isAlive());

	std::vector<std::uint32_t> ended;
	g.drainEndedSourceIds(ended);
	REQUIRE(ended.size() == 1);
}

// --- Status effect plumbing ---

TEST_CASE("Effect::slow halves the player's movement multiplier")
{
	const Effect slow = Effect::slow();
	REQUIRE(slow.speedMultiplier() < 1.f);
	REQUIRE(slow.effectId() == "slow");
}

TEST_CASE("A Slow-tagged hit applies the Slow effect to the player on next update")
{
	World w = makeEmptyWorld();
	Player p;

	Hitbox hit{p.getBounds(), 1, Team::Enemy, 42u};
	hit.statusOnHit = StatusEffectKind::Slow;

	p.onHit(hit);
	p.update(0.016f, w);

	bool hasSlow = false;
	for (const Effect &effect : p.activeEffects())
		if (effect.effectId() == "slow")
			hasSlow = true;
	REQUIRE(hasSlow);
}
