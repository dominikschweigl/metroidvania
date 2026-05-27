#include <catch2/catch_test_macros.hpp>

#include "../../src/combat/combat_system.h"
#include "../../src/combat/health.h"
#include "../../src/combat/hitbox.h"

#include <vector>

namespace {

constexpr sf::FloatRect rectAt(float x, float y, float w = 10.f, float h = 10.f)
{
	return {{x, y}, {w, h}};
}

} // namespace

TEST_CASE("CombatSystem: opposite teams, overlapping, alive victim — damages once")
{
	Health enemyHp{3, 3};
	std::vector<Hitbox> hits{{rectAt(0, 0), 1, Team::Player, 1}};
	std::vector<Hurtbox> hurts{{rectAt(5, 5), Team::Enemy, &enemyHp, false}};

	CombatSystem combat;
	combat.resolve(hits, hurts);

	REQUIRE(enemyHp.current == 2);
}

TEST_CASE("CombatSystem: same-team hit does not deal damage (friendly fire off)")
{
	Health friendHp{3, 3};
	std::vector<Hitbox> hits{{rectAt(0, 0), 1, Team::Player, 1}};
	std::vector<Hurtbox> hurts{{rectAt(5, 5), Team::Player, &friendHp, false}};

	CombatSystem combat;
	combat.resolve(hits, hurts);

	REQUIRE(friendHp.current == 3);
}

TEST_CASE("CombatSystem: non-overlapping boxes do not deal damage")
{
	Health enemyHp{3, 3};
	std::vector<Hitbox> hits{{rectAt(0, 0), 1, Team::Player, 1}};
	std::vector<Hurtbox> hurts{{rectAt(100, 100), Team::Enemy, &enemyHp, false}};

	CombatSystem combat;
	combat.resolve(hits, hurts);

	REQUIRE(enemyHp.current == 3);
}

TEST_CASE("CombatSystem: same (sourceId, victim) within one resolve damages once")
{
	Health enemyHp{5, 5};
	std::vector<Hitbox> hits{
	    {rectAt(0, 0), 1, Team::Player, 7},
	    {rectAt(0, 0), 1, Team::Player, 7},
	};
	std::vector<Hurtbox> hurts{{rectAt(5, 5), Team::Enemy, &enemyHp, false}};

	CombatSystem combat;
	combat.resolve(hits, hurts);

	REQUIRE(enemyHp.current == 4);
}

TEST_CASE("CombatSystem: same source across frames damages once until clearSource()")
{
	Health enemyHp{5, 5};
	std::vector<Hitbox> hits{{rectAt(0, 0), 1, Team::Player, 7}};
	std::vector<Hurtbox> hurts{{rectAt(5, 5), Team::Enemy, &enemyHp, false}};

	CombatSystem combat;
	combat.resolve(hits, hurts);
	combat.resolve(hits, hurts);
	combat.resolve(hits, hurts);

	REQUIRE(enemyHp.current == 4);

	combat.clearSource(7);
	combat.resolve(hits, hurts);
	REQUIRE(enemyHp.current == 3);
}

TEST_CASE("CombatSystem: distinct sourceIds each damage the same victim once")
{
	Health enemyHp{5, 5};
	std::vector<Hitbox> hits1{{rectAt(0, 0), 1, Team::Player, 1}};
	std::vector<Hitbox> hits2{{rectAt(0, 0), 1, Team::Player, 2}};
	std::vector<Hurtbox> hurts{{rectAt(5, 5), Team::Enemy, &enemyHp, false}};

	CombatSystem combat;
	combat.resolve(hits1, hurts);
	combat.resolve(hits2, hurts);

	REQUIRE(enemyHp.current == 3);
}

TEST_CASE("CombatSystem: invulnerable victim takes no damage and is not recorded")
{
	Health playerHp{5, 5};
	std::vector<Hitbox> hits{{rectAt(0, 0), 1, Team::Enemy, 9}};
	std::vector<Hurtbox> hurts{{rectAt(5, 5), Team::Player, &playerHp, true}};

	CombatSystem combat;
	combat.resolve(hits, hurts);
	REQUIRE(playerHp.current == 5);

	// When invulnerability ends, the same sourceId may damage the same victim.
	std::vector<Hurtbox> hurts_vulnerable{{rectAt(5, 5), Team::Player, &playerHp, false}};
	combat.resolve(hits, hurts_vulnerable);
	REQUIRE(playerHp.current == 4);
}

TEST_CASE("CombatSystem: dead victim takes no further damage but stays at zero (boundary)")
{
	Health enemyHp{3, 0};
	std::vector<Hitbox> hits{{rectAt(0, 0), 5, Team::Player, 1}};
	std::vector<Hurtbox> hurts{{rectAt(5, 5), Team::Enemy, &enemyHp, false}};

	CombatSystem combat;
	combat.resolve(hits, hurts);

	REQUIRE(enemyHp.current == 0);
	REQUIRE_FALSE(enemyHp.isAlive());
}

TEST_CASE("CombatSystem: zero-damage hit produces no health change (boundary)")
{
	Health enemyHp{3, 3};
	std::vector<Hitbox> hits{{rectAt(0, 0), 0, Team::Player, 1}};
	std::vector<Hurtbox> hurts{{rectAt(5, 5), Team::Enemy, &enemyHp, false}};

	CombatSystem combat;
	combat.resolve(hits, hurts);

	REQUIRE(enemyHp.current == 3);
}

TEST_CASE("CombatSystem: one hitbox damages every overlapping opposite-team hurtbox")
{
	Health a{3, 3};
	Health b{3, 3};
	std::vector<Hitbox> hits{{rectAt(0, 0, 100, 100), 1, Team::Player, 1}};
	std::vector<Hurtbox> hurts{
	    {rectAt(5, 5), Team::Enemy, &a, false},
	    {rectAt(20, 20), Team::Enemy, &b, false},
	};

	CombatSystem combat;
	combat.resolve(hits, hurts);

	REQUIRE(a.current == 2);
	REQUIRE(b.current == 2);
}

TEST_CASE("CombatSystem: empty inputs are safe (failure / boundary)")
{
	Health enemyHp{3, 3};
	std::vector<Hitbox> noHits;
	std::vector<Hurtbox> noHurts;
	std::vector<Hitbox> hits{{rectAt(0, 0), 1, Team::Player, 1}};
	std::vector<Hurtbox> hurts{{rectAt(5, 5), Team::Enemy, &enemyHp, false}};

	CombatSystem combat;
	combat.resolve(noHits, hurts);
	combat.resolve(hits, noHurts);
	combat.resolve(noHits, noHurts);

	REQUIRE(enemyHp.current == 3);
}

TEST_CASE("CombatSystem: null health pointer in hurtbox is ignored (defensive boundary)")
{
	std::vector<Hitbox> hits{{rectAt(0, 0), 1, Team::Player, 1}};
	std::vector<Hurtbox> hurts{{rectAt(5, 5), Team::Enemy, nullptr, false}};

	CombatSystem combat;
	REQUIRE_NOTHROW(combat.resolve(hits, hurts));
}

TEST_CASE("CombatSystem: clear() wipes all recorded hits")
{
	Health enemyHp{5, 5};
	std::vector<Hitbox> hits{{rectAt(0, 0), 1, Team::Player, 7}};
	std::vector<Hurtbox> hurts{{rectAt(5, 5), Team::Enemy, &enemyHp, false}};

	CombatSystem combat;
	combat.resolve(hits, hurts);
	combat.resolve(hits, hurts);
	REQUIRE(enemyHp.current == 4);

	combat.clear();
	combat.resolve(hits, hurts);
	REQUIRE(enemyHp.current == 3);
}
