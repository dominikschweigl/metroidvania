#include <catch2/catch_test_macros.hpp>

#include "entities/player/player.h"
#include "entities/player/states/landing_state.h"
#include "entities/player/states/pre_jump_state.h"

// Grants test-only access to Player private members via the friend relationship
// declared in player.h. Define it here so no game code depends on it.
struct PlayerTestAccess {
	static void setOnGround(Player &p, bool v) { p.isOnGround = v; }
	static void setVelocity(Player &p, sf::Vector2f v) { p.velocity = v; }
	static void setInputJump(Player &p, bool v) { p.inputJump = v; }
	static void setIsSprinting(Player &p, bool v) { p.isSprinting = v; }
	static PlayerState *getCurrentState(Player &p) { return p.currentState; }
	static Player::States &getStates(Player &p) { return p.states; }
};

TEST_CASE("PlayerState transitions")
{
	Player p;
	auto &states = PlayerTestAccess::getStates(p);

	SECTION("initial state is idle")
	{
		REQUIRE(PlayerTestAccess::getCurrentState(p) == &states.idle);
	}
}

// ─── IdleState ───────────────────────────────────────────────────────────────

TEST_CASE("IdleState transitions")
{
	Player p;
	auto &states = PlayerTestAccess::getStates(p);
	states.idle.onEnter(p);

	SECTION("stays idle when grounded with no input")
	{
		PlayerTestAccess::setOnGround(p, true);
		PlayerTestAccess::setVelocity(p, {0.f, 0.f});
		PlayerTestAccess::setInputJump(p, false);
		REQUIRE(states.idle.update(0.1f, p) == &states.idle);
	}

	SECTION("transitions to preJump on jump input")
	{
		PlayerTestAccess::setOnGround(p, true);
		PlayerTestAccess::setInputJump(p, true);
		REQUIRE(states.idle.update(0.1f, p) == &states.preJump);
	}

	SECTION("transitions to walking when velocity is non-zero")
	{
		PlayerTestAccess::setOnGround(p, true);
		PlayerTestAccess::setVelocity(p, {Player::WALKING_SPEED, 0.f});
		REQUIRE(states.idle.update(0.1f, p) == &states.walking);
	}

	SECTION("transitions to running when moving and sprinting")
	{
		PlayerTestAccess::setOnGround(p, true);
		PlayerTestAccess::setVelocity(p, {Player::RUNNING_SPEED, 0.f});
		PlayerTestAccess::setIsSprinting(p, true);
		REQUIRE(states.idle.update(0.1f, p) == &states.running);
	}

	SECTION("transitions to peak when airborne")
	{
		PlayerTestAccess::setOnGround(p, false);
		REQUIRE(states.idle.update(0.1f, p) == &states.peak);
	}
}

// ─── WalkingState ─────────────────────────────────────────────────────────────

TEST_CASE("WalkingState transitions")
{
	Player p;
	auto &states = PlayerTestAccess::getStates(p);
	states.walking.onEnter(p);
	PlayerTestAccess::setOnGround(p, true);
	PlayerTestAccess::setVelocity(p, {Player::WALKING_SPEED, 0.f});

	SECTION("stays walking while grounded, moving, no sprint")
	{
		REQUIRE(states.walking.update(0.1f, p) == &states.walking);
	}

	SECTION("transitions to idle when velocity is zero")
	{
		PlayerTestAccess::setVelocity(p, {0.f, 0.f});
		REQUIRE(states.walking.update(0.1f, p) == &states.idle);
	}

	SECTION("transitions to running when sprinting")
	{
		PlayerTestAccess::setIsSprinting(p, true);
		REQUIRE(states.walking.update(0.1f, p) == &states.running);
	}

	SECTION("transitions to preJump on jump input")
	{
		PlayerTestAccess::setInputJump(p, true);
		REQUIRE(states.walking.update(0.1f, p) == &states.preJump);
	}

	SECTION("transitions to peak when airborne")
	{
		PlayerTestAccess::setOnGround(p, false);
		REQUIRE(states.walking.update(0.1f, p) == &states.peak);
	}
}

// ─── RunningState ─────────────────────────────────────────────────────────────

TEST_CASE("RunningState transitions")
{
	Player p;
	auto &states = PlayerTestAccess::getStates(p);
	states.running.onEnter(p);
	PlayerTestAccess::setOnGround(p, true);
	PlayerTestAccess::setVelocity(p, {Player::RUNNING_SPEED, 0.f});
	PlayerTestAccess::setIsSprinting(p, true);

	SECTION("stays running while grounded, moving, and sprinting")
	{
		REQUIRE(states.running.update(0.1f, p) == &states.running);
	}

	SECTION("transitions to walking when sprint released")
	{
		PlayerTestAccess::setIsSprinting(p, false);
		REQUIRE(states.running.update(0.1f, p) == &states.walking);
	}

	SECTION("transitions to idle when velocity is zero")
	{
		PlayerTestAccess::setVelocity(p, {0.f, 0.f});
		REQUIRE(states.running.update(0.1f, p) == &states.idle);
	}

	SECTION("transitions to preJump on jump input")
	{
		PlayerTestAccess::setInputJump(p, true);
		REQUIRE(states.running.update(0.1f, p) == &states.preJump);
	}

	SECTION("transitions to peak when airborne")
	{
		PlayerTestAccess::setOnGround(p, false);
		REQUIRE(states.running.update(0.1f, p) == &states.peak);
	}
}

// ─── AscendingState ───────────────────────────────────────────────────────────

TEST_CASE("AscendingState transitions")
{
	Player p;
	auto &states = PlayerTestAccess::getStates(p);
	states.ascending.onEnter(p);

	SECTION("stays ascending when velocity is strongly negative")
	{
		PlayerTestAccess::setVelocity(p, {0.f, -(Player::PEAK_THRESHOLD + 1.f)});
		REQUIRE(states.ascending.update(0.1f, p) == &states.ascending);
	}

	SECTION("transitions to peak when near apex")
	{
		PlayerTestAccess::setVelocity(p, {0.f, -(Player::PEAK_THRESHOLD - 1.f)});
		REQUIRE(states.ascending.update(0.1f, p) == &states.peak);
	}

	SECTION("transitions to peak when velocity becomes positive (e.g. due to external forces)")
	{
		PlayerTestAccess::setVelocity(p, {0.f, 1.f});
		REQUIRE(states.ascending.update(0.1f, p) == &states.peak);
	}
}

// ─── PreJumpState ─────────────────────────────────────────────────────────────

TEST_CASE("PreJumpState transitions to ascending after animation completes")
{
	Player p;
	auto &states = PlayerTestAccess::getStates(p);
	states.preJump.onEnter(p);

	SECTION("stays in preJump before animation finishes")
	{
		REQUIRE(states.preJump.update(0.1f, p) == &states.preJump);
	}

	SECTION("transitions to ascending once two animation frames have elapsed")
	{
		states.preJump.applyAnimation(PreJumpState::PREJUMP_FRAME_DURATION, p);
		states.preJump.applyAnimation(PreJumpState::PREJUMP_FRAME_DURATION, p);
		REQUIRE(states.preJump.update(0.1f, p) == &states.ascending);
	}

	SECTION("readyToAscend resets on re-enter")
	{
		states.preJump.applyAnimation(PreJumpState::PREJUMP_FRAME_DURATION, p);
		states.preJump.applyAnimation(PreJumpState::PREJUMP_FRAME_DURATION, p);
		states.preJump.onEnter(p); // simulate re-entering the state
		REQUIRE(states.preJump.update(0.1f, p) == &states.preJump);
	}
}

// ─── LandingState ─────────────────────────────────────────────────────────────

TEST_CASE("LandingState transitions")
{
	Player p;
	auto &states = PlayerTestAccess::getStates(p);
	states.landing.onEnter(p);

	SECTION("immediately transitions to preJump when jump is pressed")
	{
		PlayerTestAccess::setInputJump(p, true);
		REQUIRE(states.landing.update(LandingState::LAND_FRAME_DURATION - 0.1f, p) == &states.preJump);
	}

	SECTION("stays in landing while animation is in progress and no jump input")
	{
		PlayerTestAccess::setInputJump(p, false);
		REQUIRE(states.landing.update(LandingState::LAND_FRAME_DURATION - 0.1f, p) == &states.landing);
	}

	SECTION("transitions to idle after full landing animation")
	{
		PlayerTestAccess::setInputJump(p, false);
		for (int i = 0; i < 4; ++i)
			states.landing.applyAnimation(LandingState::LAND_FRAME_DURATION, p);
		REQUIRE(states.landing.update(0.1f, p) == &states.idle);
	}

	SECTION("animationComplete resets on re-enter")
	{
		for (int i = 0; i < 4; ++i)
			states.landing.applyAnimation(LandingState::LAND_FRAME_DURATION, p);
		states.landing.onEnter(p);
		PlayerTestAccess::setInputJump(p, false);
		REQUIRE(states.landing.update(0.1f, p) == &states.landing);
	}
}
