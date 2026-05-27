#include <catch2/catch_test_macros.hpp>

#include "../../src/combat/health.h"

TEST_CASE("Health: default-constructed with positive max is alive at full HP")
{
	Health h{5, 5};
	REQUIRE(h.max == 5);
	REQUIRE(h.current == 5);
	REQUIRE(h.isAlive());
}

TEST_CASE("Health: damage(0) leaves current unchanged (boundary)")
{
	Health h{5, 5};
	h.damage(0);
	REQUIRE(h.current == 5);
	REQUIRE(h.isAlive());
}

TEST_CASE("Health: damage reduces current by exactly the given amount")
{
	Health h{5, 5};
	h.damage(2);
	REQUIRE(h.current == 3);
}

TEST_CASE("Health: overkill damage clamps current to 0 (no negative HP)")
{
	Health h{5, 5};
	h.damage(100);
	REQUIRE(h.current == 0);
	REQUIRE_FALSE(h.isAlive());
}

TEST_CASE("Health: exactly fatal damage transitions isAlive false at the boundary")
{
	Health h{5, 5};
	h.damage(4);
	REQUIRE(h.isAlive());
	REQUIRE(h.current == 1);
	h.damage(1);
	REQUIRE_FALSE(h.isAlive());
	REQUIRE(h.current == 0);
}

TEST_CASE("Health: negative damage is rejected (failure case — must not heal)")
{
	Health h{5, 3};
	h.damage(-10);
	REQUIRE(h.current == 3);
}

TEST_CASE("Health: heal restores HP up to but not past max")
{
	Health h{5, 1};
	h.heal(2);
	REQUIRE(h.current == 3);
	h.heal(99);
	REQUIRE(h.current == 5);
}

TEST_CASE("Health: heal(0) is a no-op (boundary)")
{
	Health h{5, 3};
	h.heal(0);
	REQUIRE(h.current == 3);
}

TEST_CASE("Health: negative heal is rejected (failure case — must not damage)")
{
	Health h{5, 3};
	h.heal(-10);
	REQUIRE(h.current == 3);
}

TEST_CASE("Health: zero-max Health is never alive (boundary)")
{
	Health h{0, 0};
	REQUIRE_FALSE(h.isAlive());
	h.heal(5);
	REQUIRE(h.current == 0);
	REQUIRE_FALSE(h.isAlive());
}

TEST_CASE("Health: dead entity cannot take more damage below zero (boundary)")
{
	Health h{5, 0};
	REQUIRE_FALSE(h.isAlive());
	h.damage(3);
	REQUIRE(h.current == 0);
}
