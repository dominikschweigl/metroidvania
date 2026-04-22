#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "entities/base/base_enemy.h"
#include "entities/race_condition_slime/race_condition_slime.h"
#include "world/world.h"

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
	REQUIRE(s.getState() == &s.states.idle);
	REQUIRE(s.getPosition() == groundSpawn());
	REQUIRE(s.getVelocity() == sf::Vector2f{0.f, 0.f});
	REQUIRE(s.getAttackCooldown() == 0.f);
}

TEST_CASE("RaceConditionSlime::isAttacking only true during AttackState")
{
	RaceConditionSlime s(groundSpawn());

	REQUIRE_FALSE(s.isAttacking());

	s.setState(&s.states.windup);
	REQUIRE_FALSE(s.isAttacking());

	s.setState(&s.states.attack);
	REQUIRE(s.isAttacking());

	s.setState(&s.states.recover);
	REQUIRE_FALSE(s.isAttacking());
}

TEST_CASE("onPreUpdate decrements per-frame cooldown timers (via BaseEnemy::update)")
{
	World w = makeOpenWorld();
	RaceConditionSlime s(groundSpawn());

	s.setAttackCooldown(1.0f);
	s.setJumpCooldown(0.8f);
	float telInit = s.getTeleportTimer();

	// One physics tick. Player placed far away so state stays idle.
	s.update(0.1f, w, {0.f, 0.f});

	REQUIRE(s.getAttackCooldown() == 0.9f);
	REQUIRE(s.getJumpCooldown() == Catch::Approx(0.7f));
	REQUIRE(s.getTeleportTimer() == Catch::Approx(telInit - 0.1f));
}

TEST_CASE("cooldown timers clamp at zero, do not go negative")
{
	World w = makeOpenWorld();
	RaceConditionSlime s(groundSpawn());

	s.setAttackCooldown(0.05f);
	s.setJumpCooldown(0.05f);

	s.update(0.5f, w, {0.f, 0.f});

	REQUIRE(s.getAttackCooldown() == 0.f);
	REQUIRE(s.getJumpCooldown() == 0.f);
}

TEST_CASE("tryJumpTowards no-ops when airborne")
{
	RaceConditionSlime s(groundSpawn());
	s.setOnGround(false);

	s.tryJumpTowards(200.f);
	REQUIRE(s.getVelocity().y == 0.f);
}

TEST_CASE("tryJumpTowards no-ops when height difference is below threshold")
{
	RaceConditionSlime s(groundSpawn());
	s.setOnGround(true);

	s.tryJumpTowards(RaceConditionSlime::JUMP_THRESHOLD - 1.f);
	REQUIRE(s.getVelocity().y == 0.f);
}

TEST_CASE("tryJumpTowards no-ops while jump cooldown is active")
{
	RaceConditionSlime s(groundSpawn());
	s.setOnGround(true);
	s.setJumpCooldown(0.5f);

	s.tryJumpTowards(RaceConditionSlime::JUMP_THRESHOLD + 100.f);
	REQUIRE(s.getVelocity().y == 0.f);
}

TEST_CASE("tryJumpTowards jumps when all preconditions met and sets cooldown")
{
	RaceConditionSlime s(groundSpawn());
	s.setOnGround(true);
	s.setJumpCooldown(0.f);

	s.tryJumpTowards(RaceConditionSlime::JUMP_THRESHOLD + 100.f);

	REQUIRE(s.getVelocity().y < 0.f);
	REQUIRE_FALSE(s.isOnGroundFlag());
	REQUIRE(s.getJumpCooldown() == RaceConditionSlime::JUMP_COOLDOWN);
}

TEST_CASE("tryJumpTowards caps jump speed at MAX_JUMP_SPEED for very tall targets")
{
	RaceConditionSlime s(groundSpawn());
	s.setOnGround(true);

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
		s.setVelocity({200.f, 0.f});
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
	s.setTeleportTimer(10.f);

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
	s.setTeleportTimer(10.f);

	sf::Vector2f midRange = s.getPosition() + sf::Vector2f{RaceConditionSlime::ATTACK_RANGE + 100.f, 0.f};

	SECTION("facing right → positive x velocity")
	{
		s.setFacing(BaseEnemy::Direction::Right);
		(void)s.states.chase.update(0.016f, s, w, midRange);
		REQUIRE(s.getVelocity().x == RaceConditionSlime::MOVE_SPEED);
	}

	SECTION("facing left → negative x velocity")
	{
		s.setFacing(BaseEnemy::Direction::Left);
		(void)s.states.chase.update(0.016f, s, w, midRange);
		REQUIRE(s.getVelocity().x == -RaceConditionSlime::MOVE_SPEED);
	}
}

TEST_CASE("Slime ChaseState zeros velocity when entering melee range or losing sight")
{
	World w = makeOpenWorld();
	RaceConditionSlime s(groundSpawn());
	s.states.chase.onEnter(s);
	s.setTeleportTimer(10.f);

	SECTION("melee range with cooldown → velocity zero")
	{
		s.setAttackCooldown(1.f);
		s.setVelocity({RaceConditionSlime::MOVE_SPEED, 0.f});
		sf::Vector2f melee = s.getPosition() + sf::Vector2f{RaceConditionSlime::ATTACK_RANGE - 5.f, 0.f};
		(void)s.states.chase.update(0.016f, s, w, melee);
		REQUIRE(s.getVelocity().x == 0.f);
	}

	SECTION("beyond lose range → velocity zero")
	{
		s.setVelocity({RaceConditionSlime::MOVE_SPEED, 0.f});
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
	s.setTeleportTimer(0.f);
	sf::Vector2f midRange = s.getPosition() + sf::Vector2f{RaceConditionSlime::ATTACK_RANGE + 100.f, 0.f};

	(void)s.states.chase.update(0.016f, s, w, midRange);

	REQUIRE(s.getTeleportTimer() > 0.f);
}

TEST_CASE("Slime ChaseState initiates a jump when the player is above and preconditions are met")
{
	World w = makeOpenWorld();
	RaceConditionSlime s(groundSpawn());
	s.states.chase.onEnter(s);

	// Defuse teleport, arm jump prerequisites.
	s.setTeleportTimer(10.f);
	s.setJumpCooldown(0.f);
	s.setOnGround(true);

	// Player above and horizontally in pursuit range (not melee, not lost).
	// chase.update computes heightDiff = slime.y - player.y, so player.y = slime.y - 200 → heightDiff = 200.
	sf::Vector2f aboveAndAhead = {s.getPosition().x + 100.f, s.getPosition().y - 200.f};
	REQUIRE(s.states.chase.update(0.016f, s, w, aboveAndAhead) == &s.states.chase);

	REQUIRE(s.getVelocity().y < 0.f);
	REQUIRE(s.getJumpCooldown() == RaceConditionSlime::JUMP_COOLDOWN);
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
	s.setVelocity({150.f, 0.f});
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
		s.setVelocity({200.f, 0.f});
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
		s.setVelocity({200.f, 0.f});
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
	s.setTeleportTimer(100.f);

	auto &states = s.states;

	// 1. Idle: Chase when the player appears in detect range.
	sf::Vector2f nearby = s.getPosition() + sf::Vector2f{RaceConditionSlime::DETECT_RANGE - 10.f, 0.f};
	s.update(0.016f, w, nearby);
	REQUIRE(s.getState() == &states.chase);

	// 2. Chase: WindUp when player closes to melee range and cooldown is clear.
	sf::Vector2f melee = s.getPosition() + sf::Vector2f{RaceConditionSlime::ATTACK_RANGE - 5.f, 0.f};
	s.update(0.016f, w, melee);
	REQUIRE(s.getState() == &states.windup);

	// 3. WindUp: Attack after WINDUP_DUR.
	s.update(RaceConditionSlime::WINDUP_DUR + 0.01f, w, melee);
	REQUIRE(s.getState() == &states.attack);

	// 4. Attack: Recover after ATTACK_DUR.
	s.update(RaceConditionSlime::ATTACK_DUR + 0.01f, w, melee);
	REQUIRE(s.getState() == &states.recover);

	// 5. Recover: Chase if player is still close.
	s.update(RaceConditionSlime::RECOVER_DUR + 0.01f, w, melee);
	REQUIRE(s.getState() == &states.chase);
}

TEST_CASE("After the attack flow completes and the player flees, slime returns to idle")
{
	World w = makeOpenWorld();
	RaceConditionSlime s(groundSpawn());
	s.setTeleportTimer(100.f);
	s.setState(&s.states.recover);
	s.states.recover.onEnter(s);

	// Player disappears to the far edge before recover ends.
	sf::Vector2f far = s.getPosition() + sf::Vector2f{RaceConditionSlime::DETECT_RANGE + 100.f, 0.f};
	s.update(RaceConditionSlime::RECOVER_DUR + 0.01f, w, far);
	REQUIRE(s.getState() == &s.states.idle);
}

TEST_CASE("RaceConditionSlime jump height respects custom gravity")
{
	World w = makeOpenWorld();

	SECTION("Higher gravity produces lower jumps")
	{
		RaceConditionSlime heavy(groundSpawn());
		heavy.gravity = 1800.f; // 1.5x the default
		heavy.setOnGround(true);

		// Height difference must exceed JUMP_THRESHOLD (40) to trigger jump
		// v = sqrt(2 * g * (h + ENTITY_HEIGHT)) = sqrt(2 * 1800 * 78) ≈ 529.91
		float heightDiff = 50.f;
		heavy.tryJumpTowards(heightDiff);

		float heavyJumpSpeed = std::abs(heavy.getVelocity().y);
		REQUIRE(heavyJumpSpeed == Catch::Approx(529.91f).margin(1.f));
	}

	SECTION("Lower gravity produces lower jumps")
	{
		RaceConditionSlime light(groundSpawn());
		light.gravity = 600.f; // 0.5x the default
		light.setOnGround(true);

		// Same height difference as heavy slime for comparison
		// v = sqrt(2 * g * (h + ENTITY_HEIGHT)) = sqrt(2 * 600 * 78) ≈ 305.94
		float heightDiff = 50.f;
		light.tryJumpTowards(heightDiff);

		float lightJumpSpeed = std::abs(light.getVelocity().y);
		REQUIRE(lightJumpSpeed == Catch::Approx(305.94f).margin(1.f));
	}

	SECTION("Different gravity values produce proportionally different jumps")
	{
		RaceConditionSlime s1(groundSpawn());
		s1.gravity = 1200.f;
		s1.setOnGround(true);
		s1.tryJumpTowards(20.f); // Small height to avoid cap
		float normalJump = std::abs(s1.getVelocity().y);

		RaceConditionSlime s2({groundSpawn().x + 100.f, groundSpawn().y});
		s2.gravity = 2400.f; // 2x gravity
		s2.setOnGround(true);
		s2.tryJumpTowards(20.f);
		float doubleGravityJump = std::abs(s2.getVelocity().y);

		// Jump speed should scale with sqrt(gravity)
		// sqrt(2400/1200) = sqrt(2) ≈ 1.414
		REQUIRE(doubleGravityJump == Catch::Approx(normalJump * 1.414f).margin(2.f));
	}
}

TEST_CASE("Multiple slimes can have different gravity values")
{
	World w = makeOpenWorld();

	// Spawn all slimes well above the ground so gravity applies (floor is at row 15)
	sf::Vector2f airSpawn{20.f * TILE, 5.f * TILE};

	RaceConditionSlime light(airSpawn);
	light.gravity = 800.f;

	RaceConditionSlime normal({airSpawn.x + 100.f, airSpawn.y});
	normal.gravity = 1200.f;

	RaceConditionSlime heavy({airSpawn.x + 200.f, airSpawn.y});
	heavy.gravity = 1800.f;

	// Verify each maintains its own gravity
	REQUIRE(light.gravity == 800.f);
	REQUIRE(normal.gravity == 1200.f);
	REQUIRE(heavy.gravity == 1800.f);

	// Ensure all start airborne (not on ground)
	light.setOnGround(false);
	normal.setOnGround(false);
	heavy.setOnGround(false);

	// Update and verify different falling speeds
	light.update(0.1f, w, {0.f, 0.f});
	normal.update(0.1f, w, {0.f, 0.f});
	heavy.update(0.1f, w, {0.f, 0.f});

	float lightVel = light.getVelocity().y;
	float normalVel = normal.getVelocity().y;
	float heavyVel = heavy.getVelocity().y;

	// gravity * dt = vel, so: 800*0.1=80, 1200*0.1=120, 1800*0.1=180
	REQUIRE(lightVel == Catch::Approx(80.f).margin(0.1f));
	REQUIRE(normalVel == Catch::Approx(120.f).margin(0.1f));
	REQUIRE(heavyVel == Catch::Approx(180.f).margin(0.1f));

	// Verify the ratio
	REQUIRE(heavyVel > normalVel);
	REQUIRE(normalVel > lightVel);
}
