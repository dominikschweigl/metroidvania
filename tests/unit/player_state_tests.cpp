#include <catch2/catch_test_macros.hpp>

#include "entities/player/player.h"
#include "entities/player/states/landing_state.h"
#include "entities/player/states/pre_jump_state.h"
#include "entities/player/states/wall_slide_state.h"
#include "items/chewing_gum_item.h"
#include "items/hat_item.h"
#include "world/world.h"
#include <vector>

namespace {

World makeEmptyWorld()
{
	std::vector<std::vector<int>> grid(5, std::vector<int>(10, 0));
	World world = World("test");
	world.loadFromGrid(grid);
	return world;
}

} // namespace

// Grants test-only access to Player private members via the friend relationship
// declared in player.h. Define it here so no game code depends on it.
struct PlayerTestAccess {
	static void setOnGround(Player &p, bool v) { p.isOnGround = v; }
	static void setVelocity(Player &p, sf::Vector2f v) { p.velocity = v; }
	static void setInputJump(Player &p, bool v) { p.inputJump = v; }
	static void setInputLeft(Player &p, bool v) { p.inputLeft = v; }
	static void setInputRight(Player &p, bool v) { p.inputRight = v; }
	static void setIsSprinting(Player &p, bool v) { p.isSprinting = v; }
	static void setAgainstLeftWall(Player &p, bool v) { p.isAgainstLeftWall = v; }
	static void setAgainstRightWall(Player &p, bool v) { p.isAgainstRightWall = v; }
	static sf::Vector2f getVelocity(const Player &p) { return p.velocity; }
	static float getWallJumpTimer(Player &p) { return p.wallJumpTimer; }
	static float getGravity(Player &p) { return p.gravity; }
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

	SECTION("transitions to wallSlide when pressing into left wall")
	{
		p.inventory().addItem(std::make_unique<ChewingGumItem>());
		PlayerTestAccess::setVelocity(p, {0.f, -(Player::PEAK_THRESHOLD + 1.f)});
		PlayerTestAccess::setAgainstLeftWall(p, true);
		PlayerTestAccess::setInputLeft(p, true);
		REQUIRE(states.ascending.update(0.1f, p) == &states.wallSlide);
	}

	SECTION("transitions to wallSlide when pressing into right wall")
	{
		p.inventory().addItem(std::make_unique<ChewingGumItem>());
		PlayerTestAccess::setVelocity(p, {0.f, -(Player::PEAK_THRESHOLD + 1.f)});
		PlayerTestAccess::setAgainstRightWall(p, true);
		PlayerTestAccess::setInputRight(p, true);
		REQUIRE(states.ascending.update(0.1f, p) == &states.wallSlide);
	}

	SECTION("does not transition to wallSlide when against wall but not pressing into it")
	{
		PlayerTestAccess::setVelocity(p, {0.f, -(Player::PEAK_THRESHOLD + 1.f)});
		PlayerTestAccess::setAgainstLeftWall(p, true);
		PlayerTestAccess::setInputLeft(p, false);
		REQUIRE(states.ascending.update(0.1f, p) == &states.ascending);
	}

	SECTION("does not transition to wallSlide on left wall without gum")
	{
		PlayerTestAccess::setVelocity(p, {0.f, -(Player::PEAK_THRESHOLD + 1.f)});
		PlayerTestAccess::setAgainstLeftWall(p, true);
		PlayerTestAccess::setInputLeft(p, true);
		REQUIRE(states.ascending.update(0.1f, p) == &states.ascending);
	}

	SECTION("does not transition to wallSlide on right wall without gum")
	{
		PlayerTestAccess::setVelocity(p, {0.f, -(Player::PEAK_THRESHOLD + 1.f)});
		PlayerTestAccess::setAgainstRightWall(p, true);
		PlayerTestAccess::setInputRight(p, true);
		REQUIRE(states.ascending.update(0.1f, p) == &states.ascending);
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

// ─── PeakState / DescendingState wall slide entry ─────────────────────────────

TEST_CASE("PeakState transitions to wallSlide when pressing into a wall")
{
	Player p;
	p.inventory().addItem(std::make_unique<ChewingGumItem>());
	auto &states = PlayerTestAccess::getStates(p);
	states.peak.onEnter(p);

	SECTION("transitions to wallSlide on left wall with left input")
	{
		PlayerTestAccess::setAgainstLeftWall(p, true);
		PlayerTestAccess::setInputLeft(p, true);
		REQUIRE(states.peak.update(0.1f, p) == &states.wallSlide);
	}

	SECTION("transitions to wallSlide on right wall with right input")
	{
		PlayerTestAccess::setAgainstRightWall(p, true);
		PlayerTestAccess::setInputRight(p, true);
		REQUIRE(states.peak.update(0.1f, p) == &states.wallSlide);
	}
}

TEST_CASE("DescendingState transitions to wallSlide when pressing into a wall")
{
	Player p;
	p.inventory().addItem(std::make_unique<ChewingGumItem>());
	auto &states = PlayerTestAccess::getStates(p);
	states.descending.onEnter(p);

	SECTION("transitions to wallSlide on left wall with left input")
	{
		PlayerTestAccess::setAgainstLeftWall(p, true);
		PlayerTestAccess::setInputLeft(p, true);
		REQUIRE(states.descending.update(0.1f, p) == &states.wallSlide);
	}

	SECTION("stays descending when no wall input")
	{
		PlayerTestAccess::setAgainstLeftWall(p, false);
		PlayerTestAccess::setAgainstRightWall(p, false);
		REQUIRE(states.descending.update(0.1f, p) == &states.descending);
	}
}

// ─── WallSlideState ───────────────────────────────────────────────────────────

TEST_CASE("WallSlideState transitions")
{
	Player p;
	auto &states = PlayerTestAccess::getStates(p);

	SECTION("wall jump on left wall launches right and sets wallJumpTimer")
	{
		PlayerTestAccess::setAgainstLeftWall(p, true);
		PlayerTestAccess::setAgainstRightWall(p, false);
		states.wallSlide.onEnter(p);
		PlayerTestAccess::setInputJump(p, true);
		REQUIRE(states.wallSlide.update(0.1f, p) == &states.ascending);
		REQUIRE(PlayerTestAccess::getVelocity(p).x == Player::RUNNING_SPEED);
		REQUIRE(PlayerTestAccess::getWallJumpTimer(p) == Player::WALL_JUMP_DURATION);
	}

	SECTION("wall jump on right wall launches left and sets wallJumpTimer")
	{
		PlayerTestAccess::setAgainstLeftWall(p, false);
		PlayerTestAccess::setAgainstRightWall(p, true);
		states.wallSlide.onEnter(p);
		PlayerTestAccess::setInputJump(p, true);
		REQUIRE(states.wallSlide.update(0.1f, p) == &states.ascending);
		REQUIRE(PlayerTestAccess::getVelocity(p).x == -Player::RUNNING_SPEED);
		REQUIRE(PlayerTestAccess::getWallJumpTimer(p) == Player::WALL_JUMP_DURATION);
	}

	SECTION("exits to descending when no longer against wall")
	{
		PlayerTestAccess::setAgainstLeftWall(p, true);
		states.wallSlide.onEnter(p);
		PlayerTestAccess::setAgainstLeftWall(p, false);
		PlayerTestAccess::setInputLeft(p, true);
		REQUIRE(states.wallSlide.update(0.1f, p) == &states.descending);
	}

	SECTION("exits to descending when direction key released")
	{
		PlayerTestAccess::setAgainstLeftWall(p, true);
		states.wallSlide.onEnter(p);
		PlayerTestAccess::setInputLeft(p, false);
		REQUIRE(states.wallSlide.update(0.1f, p) == &states.descending);
	}

	SECTION("stays in wallSlide while against wall with input held")
	{
		PlayerTestAccess::setAgainstLeftWall(p, true);
		states.wallSlide.onEnter(p);
		PlayerTestAccess::setInputLeft(p, true);
		PlayerTestAccess::setInputJump(p, false);
		REQUIRE(states.wallSlide.update(0.1f, p) == &states.wallSlide);
	}

	SECTION("onExit restores gravity")
	{
		PlayerTestAccess::setAgainstLeftWall(p, true);
		const float gravityBefore = PlayerTestAccess::getGravity(p);
		states.wallSlide.onEnter(p);
		REQUIRE(PlayerTestAccess::getGravity(p) < gravityBefore);
		states.wallSlide.onExit(p);
		REQUIRE(PlayerTestAccess::getGravity(p) == gravityBefore);
	}

	SECTION("onEnter diminishes upward velocity from ascending")
	{
		PlayerTestAccess::setAgainstLeftWall(p, true);
		PlayerTestAccess::setVelocity(p, {0.f, -Player::JUMP_SPEED});
		states.wallSlide.onEnter(p);
		REQUIRE(PlayerTestAccess::getVelocity(p).y > -Player::JUMP_SPEED);
	}

	SECTION("onEnter diminishes fast downward velocity")
	{
		sf::Vector2f originalVelocity = {0.f, 800.f};
		PlayerTestAccess::setAgainstLeftWall(p, true);
		PlayerTestAccess::setVelocity(p, originalVelocity);
		states.wallSlide.onEnter(p);
		REQUIRE(PlayerTestAccess::getVelocity(p).y < originalVelocity.y);
	}
}

// ─── Wall slide gating: no gum ────────────────────────────────────────────────

TEST_CASE("PeakState does not transition to wallSlide without gum")
{
	Player p;
	auto &states = PlayerTestAccess::getStates(p);
	states.peak.onEnter(p);

	SECTION("stays in peak on left wall without gum")
	{
		PlayerTestAccess::setAgainstLeftWall(p, true);
		PlayerTestAccess::setInputLeft(p, true);
		REQUIRE(states.peak.update(0.1f, p) == &states.peak);
	}

	SECTION("stays in peak on right wall without gum")
	{
		PlayerTestAccess::setAgainstRightWall(p, true);
		PlayerTestAccess::setInputRight(p, true);
		REQUIRE(states.peak.update(0.1f, p) == &states.peak);
	}
}

TEST_CASE("DescendingState does not transition to wallSlide without gum")
{
	Player p;
	auto &states = PlayerTestAccess::getStates(p);
	states.descending.onEnter(p);

	SECTION("stays descending on left wall without gum")
	{
		PlayerTestAccess::setAgainstLeftWall(p, true);
		PlayerTestAccess::setInputLeft(p, true);
		REQUIRE(states.descending.update(0.1f, p) == &states.descending);
	}

	SECTION("stays descending on right wall without gum")
	{
		PlayerTestAccess::setAgainstRightWall(p, true);
		PlayerTestAccess::setInputRight(p, true);
		REQUIRE(states.descending.update(0.1f, p) == &states.descending);
	}
}

// ─── Hat throw gating ─────────────────────────────────────────────────────────

TEST_CASE("Player: hat throw is gated by hat in inventory")
{
	World world = makeEmptyWorld();

	SECTION("hat throw activates when hat is equipped")
	{
		Player p;
		p.inventory().addItem(std::make_unique<HatItem>());
		p.update(0.1f, world, false, true);
		REQUIRE(p.isAttackActive());
	}

	SECTION("hat throw does not activate without hat equipped")
	{
		Player p;
		p.update(0.1f, world, false, true);
		REQUIRE(!p.isAttackActive());
	}
}
