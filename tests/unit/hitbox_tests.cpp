#include <catch2/catch_test_macros.hpp>

#include "../../src/combat/hitbox.h"
#include "../../src/combat/health.h"

TEST_CASE("Teams: Player and Enemy are distinct")
{
	REQUIRE(Team::Player != Team::Enemy);
}

TEST_CASE("intersects: overlapping rectangles return true")
{
	Hitbox hit{{{0.f, 0.f}, {10.f, 10.f}}, 1, Team::Player, 1};
	Hurtbox hurt{{{5.f, 5.f}, {10.f, 10.f}}, Team::Enemy, nullptr, false};
	REQUIRE(intersects(hit, hurt));
}

TEST_CASE("intersects: fully separated rectangles return false")
{
	Hitbox hit{{{0.f, 0.f}, {10.f, 10.f}}, 1, Team::Player, 1};
	Hurtbox hurt{{{100.f, 100.f}, {10.f, 10.f}}, Team::Enemy, nullptr, false};
	REQUIRE_FALSE(intersects(hit, hurt));
}

TEST_CASE("intersects: edge-touching rectangles are not overlapping (boundary)")
{
	// Right edge of hit at x=10; left edge of hurt at x=10. Touch only, no area.
	Hitbox hit{{{0.f, 0.f}, {10.f, 10.f}}, 1, Team::Player, 1};
	Hurtbox hurt{{{10.f, 0.f}, {10.f, 10.f}}, Team::Enemy, nullptr, false};
	REQUIRE_FALSE(intersects(hit, hurt));
}

TEST_CASE("intersects: one-pixel overlap is overlapping (boundary)")
{
	Hitbox hit{{{0.f, 0.f}, {10.f, 10.f}}, 1, Team::Player, 1};
	Hurtbox hurt{{{9.f, 0.f}, {10.f, 10.f}}, Team::Enemy, nullptr, false};
	REQUIRE(intersects(hit, hurt));
}

TEST_CASE("intersects: zero-size hitbox never intersects (failure / boundary)")
{
	Hitbox hit{{{5.f, 5.f}, {0.f, 0.f}}, 1, Team::Player, 1};
	Hurtbox hurt{{{0.f, 0.f}, {10.f, 10.f}}, Team::Enemy, nullptr, false};
	REQUIRE_FALSE(intersects(hit, hurt));
}

TEST_CASE("intersects: zero-size hurtbox never intersects (failure / boundary)")
{
	Hitbox hit{{{0.f, 0.f}, {10.f, 10.f}}, 1, Team::Player, 1};
	Hurtbox hurt{{{5.f, 5.f}, {0.f, 0.f}}, Team::Enemy, nullptr, false};
	REQUIRE_FALSE(intersects(hit, hurt));
}

TEST_CASE("intersects: contained rectangle is overlapping")
{
	Hitbox hit{{{0.f, 0.f}, {100.f, 100.f}}, 1, Team::Player, 1};
	Hurtbox hurt{{{40.f, 40.f}, {10.f, 10.f}}, Team::Enemy, nullptr, false};
	REQUIRE(intersects(hit, hurt));
}

TEST_CASE("Hitbox and Hurtbox carry their declared fields verbatim")
{
	Health enemyHealth{3, 3};
	Hitbox hit{{{1.f, 2.f}, {3.f, 4.f}}, 5, Team::Player, 42};
	Hurtbox hurt{{{6.f, 7.f}, {8.f, 9.f}}, Team::Enemy, &enemyHealth, true};

	REQUIRE(hit.damage == 5);
	REQUIRE(hit.team == Team::Player);
	REQUIRE(hit.sourceId == 42u);

	REQUIRE(hurt.team == Team::Enemy);
	REQUIRE(hurt.health == &enemyHealth);
	REQUIRE(hurt.invulnerable);
}
