#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "entities/enemies/resistor_bug/resistor_bug.h"
#include "world/world.h"

#include "entities/enemies/resistor_bug/states/chase_state.h"
#include "entities/enemies/resistor_bug/states/idle_state.h"
#include "entities/enemies/resistor_bug/states/jump_attack_state.h"
#include "entities/enemies/resistor_bug/states/recover_state.h"

#include <cmath>

namespace {

constexpr float TILE = World::TILE_SIZE;

// Large open world big enough that bug physics don't immediately clip.
World makeOpenWorld()
{
	std::vector<std::vector<int>> g(20, std::vector<int>(40, 0));
	// Floor row 15.
	for (int x = 0; x < 40; ++x)
		g[15][x] = 1;
	World w = World("test");
	w.loadFromGrid(g);
	return w;
}

sf::Vector2f groundSpawn()
{
	return {20.f * TILE, 15.f * TILE};
}

} // namespace

TEST_CASE("ResistorBug starts in Idle state at spawn position")
{
	ResistorBug bug(groundSpawn());
	REQUIRE(bug.getState() == &bug.states.idle);
	REQUIRE(bug.getPosition() == groundSpawn());
	REQUIRE(bug.getVelocity() == sf::Vector2f{0.f, 0.f});
	REQUIRE(bug.getAttackCooldown() == 0.f);
	REQUIRE_FALSE(bug.isAttacking());
	REQUIRE_FALSE(bug.getHitbox().has_value());
}

// IdleState

TEST_CASE("ResistorBug IdleState transitions")
{
	World w = makeOpenWorld();
	ResistorBug bug(groundSpawn());
	bug.states.idle.onEnter(bug);

	SECTION("stays idle when player is out of detect range")
	{
		sf::Vector2f far = bug.getPosition() + sf::Vector2f{ResistorBug::DETECT_RANGE + 100.f, 0.f};
		REQUIRE(bug.states.idle.update(0.016f, bug, w, far) == &bug.states.idle);
	}

	SECTION("transitions to chase when player enters detect range")
	{
		sf::Vector2f near = bug.getPosition() + sf::Vector2f{ResistorBug::DETECT_RANGE - 10.f, 0.f};
		REQUIRE(bug.states.idle.update(0.016f, bug, w, near) == &bug.states.chase);
	}

	SECTION("stays idle while attack cooldown active and player is within attack range")
	{
		bug.setAttackCooldown(1.f);
		sf::Vector2f melee = bug.getPosition() + sf::Vector2f{ResistorBug::ATTACK_RANGE - 5.f, 0.f};
		REQUIRE(bug.states.idle.update(0.016f, bug, w, melee) == &bug.states.idle);
	}

	SECTION("zeroes horizontal velocity")
	{
		bug.setVelocity({200.f, 0.f});
		(void)bug.states.idle.update(0.016f, bug, w, bug.getPosition());
		REQUIRE(bug.getVelocity().x == 0.f);
	}
}

// ChaseState

TEST_CASE("ResistorBug ChaseState transitions")
{
	World w = makeOpenWorld();
	ResistorBug bug(groundSpawn());
	bug.states.chase.onEnter(bug);
	bug.setOnGround(true);

	SECTION("transitions to jump attack when in range, grounded, off cooldown")
	{
		bug.setAttackCooldown(0.f);
		sf::Vector2f melee = bug.getPosition() + sf::Vector2f{ResistorBug::ATTACK_RANGE - 5.f, 0.f};
		REQUIRE(bug.states.chase.update(0.016f, bug, w, melee) == &bug.states.jumpAttack);
	}

	SECTION("stays in chase when in range but cooldown is active")
	{
		bug.setAttackCooldown(1.f);
		sf::Vector2f melee = bug.getPosition() + sf::Vector2f{ResistorBug::ATTACK_RANGE - 5.f, 0.f};
		REQUIRE(bug.states.chase.update(0.016f, bug, w, melee) == &bug.states.chase);
	}

	SECTION("returns to idle when player escapes beyond lose range")
	{
		sf::Vector2f far = bug.getPosition() + sf::Vector2f{ResistorBug::LOSE_RANGE + 50.f, 0.f};
		REQUIRE(bug.states.chase.update(0.016f, bug, w, far) == &bug.states.idle);
	}

	SECTION("continues chasing while player is in detect range but out of attack range")
	{
		sf::Vector2f midRange = bug.getPosition() + sf::Vector2f{ResistorBug::ATTACK_RANGE + 100.f, 0.f};
		REQUIRE(bug.states.chase.update(0.016f, bug, w, midRange) == &bug.states.chase);
	}
}

TEST_CASE("ResistorBug ChaseState sets velocity in the facing direction when pursuing")
{
	World w = makeOpenWorld();
	ResistorBug bug(groundSpawn());
	bug.states.chase.onEnter(bug);

	sf::Vector2f midRange = bug.getPosition() + sf::Vector2f{ResistorBug::ATTACK_RANGE + 100.f, 0.f};

	SECTION("facing right -> positive x velocity")
	{
		bug.setDirection(Direction::Right);
		(void)bug.states.chase.update(0.016f, bug, w, midRange);
		REQUIRE(bug.getVelocity().x == Catch::Approx(ResistorBug::MOVE_SPEED));
	}

	SECTION("facing left -> negative x velocity")
	{
		bug.setDirection(Direction::Left);
		(void)bug.states.chase.update(0.016f, bug, w, midRange);
		REQUIRE(bug.getVelocity().x == Catch::Approx(-ResistorBug::MOVE_SPEED));
	}
}

// JumpAttackState

TEST_CASE("ResistorBug JumpAttackState telegraphs before hopping")
{
	World w = makeOpenWorld();
	ResistorBug bug(groundSpawn());
	bug.setOnGround(true);
	bug.states.jumpAttack.onEnter(bug);
	sf::Vector2f player = bug.getPosition() + sf::Vector2f{ResistorBug::ATTACK_RANGE - 5.f, 0.f};

	SECTION("onEnter starts the attack cooldown")
	{
		REQUIRE(bug.getAttackCooldown() == Catch::Approx(ResistorBug::ATTACK_COOLDOWN));
	}

	SECTION("stays stationary and non-damaging during the telegraph")
	{
		bug.setVelocity({200.f, 0.f});
		EnemyState *next = bug.states.jumpAttack.update(ResistorBug::TELEGRAPH_DUR * 0.5f, bug, w, player);
		REQUIRE(next == &bug.states.jumpAttack);
		REQUIRE(bug.getVelocity().x == 0.f);
		REQUIRE_FALSE(bug.isAttacking());
		REQUIRE_FALSE(bug.getHitbox().has_value());
	}

	SECTION("launches a hop once the telegraph completes")
	{
		EnemyState *next = bug.states.jumpAttack.update(ResistorBug::TELEGRAPH_DUR + 0.01f, bug, w, player);
		REQUIRE(next == &bug.states.jumpAttack);
		REQUIRE(bug.getVelocity().y < 0.f); // launched upward
		REQUIRE(bug.getVelocity().x > 0.f); // toward player on the right
		REQUIRE_FALSE(bug.isOnGroundFlag());
		REQUIRE(bug.isAttacking());
		REQUIRE(bug.getHitbox().has_value());
	}
}

TEST_CASE("ResistorBug JumpAttackState recovers after landing")
{
	World w = makeOpenWorld();
	ResistorBug bug(groundSpawn());
	bug.setOnGround(true);
	bug.states.jumpAttack.onEnter(bug);
	sf::Vector2f player = bug.getPosition() + sf::Vector2f{ResistorBug::ATTACK_RANGE - 5.f, 0.f};

	// Complete the telegraph + launch.
	(void)bug.states.jumpAttack.update(ResistorBug::TELEGRAPH_DUR + 0.01f, bug, w, player);
	REQUIRE(bug.isAttacking());

	// Simulate landing.
	bug.setOnGround(true);
	REQUIRE(bug.states.jumpAttack.update(0.016f, bug, w, player) == &bug.states.recover);

	// onExit must close the damage window.
	bug.states.jumpAttack.onExit(bug);
	REQUIRE_FALSE(bug.isAttacking());
	REQUIRE_FALSE(bug.getHitbox().has_value());
}

// RecoverState

TEST_CASE("ResistorBug RecoverState transitions")
{
	World w = makeOpenWorld();
	ResistorBug bug(groundSpawn());
	bug.states.recover.onEnter(bug);

	SECTION("stays in recover before the duration elapses")
	{
		REQUIRE(bug.states.recover.update(ResistorBug::RECOVER_DUR * 0.5f, bug, w, bug.getPosition())
		        == &bug.states.recover);
	}

	SECTION("returns to chase when player still in detect range")
	{
		sf::Vector2f near = bug.getPosition() + sf::Vector2f{ResistorBug::DETECT_RANGE - 10.f, 0.f};
		REQUIRE(bug.states.recover.update(ResistorBug::RECOVER_DUR + 0.01f, bug, w, near) == &bug.states.chase);
	}

	SECTION("returns to idle when player out of detect range")
	{
		sf::Vector2f far = bug.getPosition() + sf::Vector2f{ResistorBug::DETECT_RANGE + 100.f, 0.f};
		REQUIRE(bug.states.recover.update(ResistorBug::RECOVER_DUR + 0.01f, bug, w, far) == &bug.states.idle);
	}
}
