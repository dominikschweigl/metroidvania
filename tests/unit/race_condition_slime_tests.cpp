#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "enemy_test_access.h"

#include "entities/race_condition_slime/states/attack_state.h"
#include "entities/race_condition_slime/states/chase_state.h"
#include "entities/race_condition_slime/states/idle_state.h"
#include "entities/race_condition_slime/states/recover_state.h"
#include "entities/race_condition_slime/states/windup_state.h"

#include <cmath>

namespace {

constexpr float TILE = World::TILE_SIZE;

// Large open world big enough that slime physics don't immediately clip.
World makeOpenWorld()
{
	std::vector<std::vector<int>> g(20, std::vector<int>(40, 0));
	// Floor row 15.
	for (int x = 0; x < 40; ++x)
		g[15][x] = 1;
	World w;
	w.loadFromGrid(g);
	return w;
}

sf::Vector2f groundSpawn()
{
	return {20.f * TILE, 15.f * TILE};
}

} // namespace

TEST_CASE("RaceConditionSlime starts in Idle state at spawn position")
{
	RaceConditionSlime s(groundSpawn());
	REQUIRE(EnemyTestAccess::getCurrentState(s) == &s.states.idle);
	REQUIRE(s.getPosition() == groundSpawn());
	REQUIRE(s.getVelocity() == sf::Vector2f{0.f, 0.f});
	REQUIRE(s.getAttackCooldown() == 0.f);
}

TEST_CASE("RaceConditionSlime::isAttacking only true during AttackState")
{
	RaceConditionSlime s(groundSpawn());

	REQUIRE_FALSE(s.isAttacking());

	EnemyTestAccess::setCurrentState(s, &s.states.windup);
	REQUIRE_FALSE(s.isAttacking());

	EnemyTestAccess::setCurrentState(s, &s.states.attack);
	REQUIRE(s.isAttacking());

	EnemyTestAccess::setCurrentState(s, &s.states.recover);
	REQUIRE_FALSE(s.isAttacking());
}

TEST_CASE("onPreUpdate decrements per-frame cooldown timers (via BaseEnemy::update)")
{
	World w = makeOpenWorld();
	RaceConditionSlime s(groundSpawn());

	s.setAttackCooldown(1.0f);
	SlimeTestAccess::setJumpCooldown(s, 0.8f);
	float telInit = SlimeTestAccess::getTeleportTimer(s);

	// One physics tick. Player placed far away so state stays idle.
	s.update(0.1f, w, {0.f, 0.f});

	REQUIRE(s.getAttackCooldown() == 0.9f);
	REQUIRE(SlimeTestAccess::getJumpCooldown(s) == Catch::Approx(0.7f));
	REQUIRE(SlimeTestAccess::getTeleportTimer(s) == Catch::Approx(telInit - 0.1f));
}

TEST_CASE("cooldown timers clamp at zero, do not go negative")
{
	World w = makeOpenWorld();
	RaceConditionSlime s(groundSpawn());

	s.setAttackCooldown(0.05f);
	SlimeTestAccess::setJumpCooldown(s, 0.05f);

	s.update(0.5f, w, {0.f, 0.f});

	REQUIRE(s.getAttackCooldown() == 0.f);
	REQUIRE(SlimeTestAccess::getJumpCooldown(s) == 0.f);
}

TEST_CASE("tryJumpTowards no-ops when airborne")
{
	RaceConditionSlime s(groundSpawn());
	EnemyTestAccess::setOnGround(s, false);

	s.tryJumpTowards(200.f);
	REQUIRE(s.getVelocity().y == 0.f);
}

TEST_CASE("tryJumpTowards no-ops when height difference is below threshold")
{
	RaceConditionSlime s(groundSpawn());
	EnemyTestAccess::setOnGround(s, true);

	s.tryJumpTowards(RaceConditionSlime::JUMP_THRESHOLD - 1.f);
	REQUIRE(s.getVelocity().y == 0.f);
}

TEST_CASE("tryJumpTowards no-ops while jump cooldown is active")
{
	RaceConditionSlime s(groundSpawn());
	EnemyTestAccess::setOnGround(s, true);
	SlimeTestAccess::setJumpCooldown(s, 0.5f);

	s.tryJumpTowards(RaceConditionSlime::JUMP_THRESHOLD + 100.f);
	REQUIRE(s.getVelocity().y == 0.f);
}

TEST_CASE("tryJumpTowards jumps when all preconditions met and sets cooldown")
{
	RaceConditionSlime s(groundSpawn());
	EnemyTestAccess::setOnGround(s, true);
	SlimeTestAccess::setJumpCooldown(s, 0.f);

	s.tryJumpTowards(RaceConditionSlime::JUMP_THRESHOLD + 100.f);

	REQUIRE(s.getVelocity().y < 0.f);
	REQUIRE_FALSE(s.isOnGroundFlag());
	REQUIRE(SlimeTestAccess::getJumpCooldown(s) == RaceConditionSlime::JUMP_COOLDOWN);
}

TEST_CASE("tryJumpTowards caps jump speed at MAX_JUMP_SPEED for very tall targets")
{
	RaceConditionSlime s(groundSpawn());
	EnemyTestAccess::setOnGround(s, true);

	// Huge height difference would otherwise yield sqrt(2*g*h) >> MAX_JUMP_SPEED.
	s.tryJumpTowards(10000.f);
	REQUIRE(std::abs(s.getVelocity().y) == RaceConditionSlime::MAX_JUMP_SPEED);
}

// IdleState

TEST_CASE("Slime IdleState transitions")
{
	World w = makeOpenWorld();
	RaceConditionSlime s(groundSpawn());
	s.states.idle.onEnter(s);

	SECTION("stays idle when player is out of detect range")
	{
		sf::Vector2f far = s.getPosition() + sf::Vector2f{RaceConditionSlime::DETECT_RANGE + 100.f, 0.f};
		REQUIRE(s.states.idle.update(0.016f, s, w, far) == &s.states.idle);
	}

	SECTION("transitions to chase when player enters detect range")
	{
		sf::Vector2f near = s.getPosition() + sf::Vector2f{RaceConditionSlime::DETECT_RANGE - 10.f, 0.f};
		REQUIRE(s.states.idle.update(0.016f, s, w, near) == &s.states.chase);
	}

	SECTION("stays idle while attack cooldown active and player is within attack range")
	{
		s.setAttackCooldown(1.f);
		sf::Vector2f melee = s.getPosition() + sf::Vector2f{RaceConditionSlime::ATTACK_RANGE - 5.f, 0.f};
		REQUIRE(s.states.idle.update(0.016f, s, w, melee) == &s.states.idle);
	}

	SECTION("attack cooldown does not gate transition when player is beyond attack range")
	{
		s.setAttackCooldown(1.f);
		sf::Vector2f detectButNotMelee = s.getPosition() + sf::Vector2f{RaceConditionSlime::DETECT_RANGE - 10.f, 0.f};
		REQUIRE(s.states.idle.update(0.016f, s, w, detectButNotMelee) == &s.states.chase);
	}

	SECTION("zeroes horizontal velocity")
	{
		EnemyTestAccess::setVel(s, {200.f, 0.f});
		(void)s.states.idle.update(0.016f, s, w, s.getPosition());
		REQUIRE(s.getVelocity().x == 0.f);
	}
}

// ChaseState

TEST_CASE("Slime ChaseState transitions")
{
	World w = makeOpenWorld();
	RaceConditionSlime s(groundSpawn());
	s.states.chase.onEnter(s);

	// Prevent slime.maybeTeleport from firing incidentally.
	SlimeTestAccess::setTeleportTimer(s, 10.f);

	SECTION("transitions to windup when in attack range with no cooldown")
	{
		s.setAttackCooldown(0.f);
		sf::Vector2f melee = s.getPosition() + sf::Vector2f{RaceConditionSlime::ATTACK_RANGE - 5.f, 0.f};
		REQUIRE(s.states.chase.update(0.016f, s, w, melee) == &s.states.windup);
	}

	SECTION("returns to idle when in attack range but cooldown is active")
	{
		s.setAttackCooldown(1.f);
		sf::Vector2f melee = s.getPosition() + sf::Vector2f{RaceConditionSlime::ATTACK_RANGE - 5.f, 0.f};
		REQUIRE(s.states.chase.update(0.016f, s, w, melee) == &s.states.idle);
	}

	SECTION("returns to idle when player escapes beyond lose range")
	{
		sf::Vector2f far = s.getPosition() + sf::Vector2f{RaceConditionSlime::LOSE_RANGE + 50.f, 0.f};
		REQUIRE(s.states.chase.update(0.016f, s, w, far) == &s.states.idle);
	}

	SECTION("continues chasing while player is in detect range but out of melee")
	{
		sf::Vector2f midRange = s.getPosition() + sf::Vector2f{RaceConditionSlime::ATTACK_RANGE + 100.f, 0.f};
		REQUIRE(s.states.chase.update(0.016f, s, w, midRange) == &s.states.chase);
	}
}

TEST_CASE("Slime ChaseState sets velocity in the facing direction when pursuing")
{
	World w = makeOpenWorld();
	RaceConditionSlime s(groundSpawn());
	s.states.chase.onEnter(s);
	SlimeTestAccess::setTeleportTimer(s, 10.f);

	sf::Vector2f midRange = s.getPosition() + sf::Vector2f{RaceConditionSlime::ATTACK_RANGE + 100.f, 0.f};

	SECTION("facing right → positive x velocity")
	{
		EnemyTestAccess::setFacing(s, BaseEnemy::Direction::Right);
		(void)s.states.chase.update(0.016f, s, w, midRange);
		REQUIRE(s.getVelocity().x == RaceConditionSlime::MOVE_SPEED);
	}

	SECTION("facing left → negative x velocity")
	{
		EnemyTestAccess::setFacing(s, BaseEnemy::Direction::Left);
		(void)s.states.chase.update(0.016f, s, w, midRange);
		REQUIRE(s.getVelocity().x == -RaceConditionSlime::MOVE_SPEED);
	}
}

TEST_CASE("Slime ChaseState zeros velocity when entering melee range or losing sight")
{
	World w = makeOpenWorld();
	RaceConditionSlime s(groundSpawn());
	s.states.chase.onEnter(s);
	SlimeTestAccess::setTeleportTimer(s, 10.f);

	SECTION("melee range with cooldown → velocity zero")
	{
		s.setAttackCooldown(1.f);
		EnemyTestAccess::setVel(s, {RaceConditionSlime::MOVE_SPEED, 0.f});
		sf::Vector2f melee = s.getPosition() + sf::Vector2f{RaceConditionSlime::ATTACK_RANGE - 5.f, 0.f};
		(void)s.states.chase.update(0.016f, s, w, melee);
		REQUIRE(s.getVelocity().x == 0.f);
	}

	SECTION("beyond lose range → velocity zero")
	{
		EnemyTestAccess::setVel(s, {RaceConditionSlime::MOVE_SPEED, 0.f});
		sf::Vector2f far = s.getPosition() + sf::Vector2f{RaceConditionSlime::LOSE_RANGE + 50.f, 0.f};
		(void)s.states.chase.update(0.016f, s, w, far);
		REQUIRE(s.getVelocity().x == 0.f);
	}
}

TEST_CASE("Slime ChaseState triggers glitch teleport when teleport timer expires")
{
	World w = makeOpenWorld();
	RaceConditionSlime s(groundSpawn());
	s.states.chase.onEnter(s);

	// Force the teleport gate open. glitchTeleport always calls resetTeleportTimer
	// first, so the timer being positive afterwards is a reliable effect signal
	// regardless of which RNG branch fires.
	SlimeTestAccess::setTeleportTimer(s, 0.f);
	sf::Vector2f midRange = s.getPosition() + sf::Vector2f{RaceConditionSlime::ATTACK_RANGE + 100.f, 0.f};

	(void)s.states.chase.update(0.016f, s, w, midRange);

	REQUIRE(SlimeTestAccess::getTeleportTimer(s) > 0.f);
}

TEST_CASE("Slime ChaseState initiates a jump when the player is above and preconditions are met")
{
	World w = makeOpenWorld();
	RaceConditionSlime s(groundSpawn());
	s.states.chase.onEnter(s);

	// Defuse teleport, arm jump prerequisites.
	SlimeTestAccess::setTeleportTimer(s, 10.f);
	SlimeTestAccess::setJumpCooldown(s, 0.f);
	EnemyTestAccess::setOnGround(s, true);

	// Player above and horizontally in pursuit range (not melee, not lost).
	// chase.update computes heightDiff = slime.y - player.y, so player.y = slime.y - 200 → heightDiff = 200.
	sf::Vector2f aboveAndAhead = {s.getPosition().x + 100.f, s.getPosition().y - 200.f};
	REQUIRE(s.states.chase.update(0.016f, s, w, aboveAndAhead) == &s.states.chase);

	REQUIRE(s.getVelocity().y < 0.f);
	REQUIRE(SlimeTestAccess::getJumpCooldown(s) == RaceConditionSlime::JUMP_COOLDOWN);
}

// WindUpState

TEST_CASE("Slime WindUpState transitions")
{
	World w = makeOpenWorld();
	RaceConditionSlime s(groundSpawn());
	s.states.windup.onEnter(s);

	SECTION("stays in windup while under duration")
	{
		REQUIRE(s.states.windup.update(RaceConditionSlime::WINDUP_DUR * 0.25f, s, w, s.getPosition())
		        == &s.states.windup);
	}

	SECTION("transitions to attack once duration elapses")
	{
		REQUIRE(s.states.windup.update(RaceConditionSlime::WINDUP_DUR + 0.01f, s, w, s.getPosition())
		        == &s.states.attack);
	}
}

TEST_CASE("Slime WindUpState::onEnter arms the attack cooldown and zeros velocity")
{
	RaceConditionSlime s(groundSpawn());
	EnemyTestAccess::setVel(s, {150.f, 0.f});
	s.setAttackCooldown(0.f);

	s.states.windup.onEnter(s);

	REQUIRE(s.getAttackCooldown() == RaceConditionSlime::ATTACK_COOLDOWN);
	REQUIRE(s.getVelocity().x == 0.f);
}

TEST_CASE("Slime WindUpState timer resets across re-entry")
{
	World w = makeOpenWorld();
	RaceConditionSlime s(groundSpawn());
	s.states.windup.onEnter(s);
	// Tick to nearly completion.
	REQUIRE(s.states.windup.update(RaceConditionSlime::WINDUP_DUR - 0.05f, s, w, s.getPosition()) == &s.states.windup);

	// Re-enter: timer is cleared, so a small dt should not trigger transition.
	s.states.windup.onEnter(s);
	REQUIRE(s.states.windup.update(0.01f, s, w, s.getPosition()) == &s.states.windup);
}

// AttackState

TEST_CASE("Slime AttackState transitions")
{
	World w = makeOpenWorld();
	RaceConditionSlime s(groundSpawn());
	s.states.attack.onEnter(s);

	SECTION("stays in attack while under duration")
	{
		REQUIRE(s.states.attack.update(RaceConditionSlime::ATTACK_DUR * 0.5f, s, w, s.getPosition())
		        == &s.states.attack);
	}

	SECTION("transitions to recover once duration elapses")
	{
		REQUIRE(s.states.attack.update(RaceConditionSlime::ATTACK_DUR + 0.01f, s, w, s.getPosition())
		        == &s.states.recover);
	}

	SECTION("zeroes velocity each tick")
	{
		EnemyTestAccess::setVel(s, {200.f, 0.f});
		(void)s.states.attack.update(0.016f, s, w, s.getPosition());
		REQUIRE(s.getVelocity().x == 0.f);
	}
}

// RecoverState

TEST_CASE("Slime RecoverState transitions")
{
	World w = makeOpenWorld();
	RaceConditionSlime s(groundSpawn());
	s.states.recover.onEnter(s);

	SECTION("stays in recover while under duration")
	{
		REQUIRE(s.states.recover.update(RaceConditionSlime::RECOVER_DUR * 0.25f, s, w, s.getPosition())
		        == &s.states.recover);
	}

	SECTION("returns to chase when recover finishes and player is still close")
	{
		sf::Vector2f near = s.getPosition() + sf::Vector2f{RaceConditionSlime::DETECT_RANGE - 10.f, 0.f};
		REQUIRE(s.states.recover.update(RaceConditionSlime::RECOVER_DUR + 0.01f, s, w, near) == &s.states.chase);
	}

	SECTION("returns to idle when recover finishes and player has fled beyond detect range")
	{
		sf::Vector2f far = s.getPosition() + sf::Vector2f{RaceConditionSlime::DETECT_RANGE + 100.f, 0.f};
		REQUIRE(s.states.recover.update(RaceConditionSlime::RECOVER_DUR + 0.01f, s, w, far) == &s.states.idle);
	}

	SECTION("zeroes velocity while recovering")
	{
		EnemyTestAccess::setVel(s, {200.f, 0.f});
		(void)s.states.recover.update(0.016f, s, w, s.getPosition());
		REQUIRE(s.getVelocity().x == 0.f);
	}
}

// End-to-end flow through BaseEnemy::update

TEST_CASE("Full combat flow: idle -> chase -> windup -> attack -> recover")
{
	World w = makeOpenWorld();
	RaceConditionSlime s(groundSpawn());
	// Pin teleport timer high so teleports can't influence the test.
	SlimeTestAccess::setTeleportTimer(s, 100.f);

	auto &states = s.states;

	// 1. Idle: Chase when the player appears in detect range.
	sf::Vector2f nearby = s.getPosition() + sf::Vector2f{RaceConditionSlime::DETECT_RANGE - 10.f, 0.f};
	s.update(0.016f, w, nearby);
	REQUIRE(EnemyTestAccess::getCurrentState(s) == &states.chase);

	// 2. Chase: WindUp when player closes to melee range and cooldown is clear.
	sf::Vector2f melee = s.getPosition() + sf::Vector2f{RaceConditionSlime::ATTACK_RANGE - 5.f, 0.f};
	s.update(0.016f, w, melee);
	REQUIRE(EnemyTestAccess::getCurrentState(s) == &states.windup);

	// 3. WindUp: Attack after WINDUP_DUR.
	s.update(RaceConditionSlime::WINDUP_DUR + 0.01f, w, melee);
	REQUIRE(EnemyTestAccess::getCurrentState(s) == &states.attack);

	// 4. Attack: Recover after ATTACK_DUR.
	s.update(RaceConditionSlime::ATTACK_DUR + 0.01f, w, melee);
	REQUIRE(EnemyTestAccess::getCurrentState(s) == &states.recover);

	// 5. Recover: Chase if player is still close.
	s.update(RaceConditionSlime::RECOVER_DUR + 0.01f, w, melee);
	REQUIRE(EnemyTestAccess::getCurrentState(s) == &states.chase);
}

TEST_CASE("After the attack flow completes and the player flees, slime returns to idle")
{
	World w = makeOpenWorld();
	RaceConditionSlime s(groundSpawn());
	SlimeTestAccess::setTeleportTimer(s, 100.f);
	EnemyTestAccess::setCurrentState(s, &s.states.recover);
	s.states.recover.onEnter(s);

	// Player disappears to the far edge before recover ends.
	sf::Vector2f far = s.getPosition() + sf::Vector2f{RaceConditionSlime::DETECT_RANGE + 100.f, 0.f};
	s.update(RaceConditionSlime::RECOVER_DUR + 0.01f, w, far);
	REQUIRE(EnemyTestAccess::getCurrentState(s) == &s.states.idle);
}
