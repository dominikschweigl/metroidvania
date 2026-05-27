#include <catch2/catch_test_macros.hpp>

#include "../../src/combat/hitbox.h"
#include "../../src/entities/direction.h"
#include "../../src/entities/player/abilities/melee_attack.h"

TEST_CASE("MeleeAttack: getHitbox is empty before any trigger")
{
	MeleeAttack atk;
	REQUIRE_FALSE(atk.getHitbox({100.f, 100.f}, Direction::Right).has_value());
}

TEST_CASE("MeleeAttack: getHitbox is present while a swing is active")
{
	MeleeAttack atk;
	atk.trigger();

	const auto hit = atk.getHitbox({100.f, 100.f}, Direction::Right);
	REQUIRE(hit.has_value());
	REQUIRE(hit->team == Team::Player);
	REQUIRE(hit->damage == MeleeAttack::DAMAGE);
	REQUIRE(hit->damage > 0);
	REQUIRE(hit->sourceId != 0u);
}

TEST_CASE("MeleeAttack: getHitbox is empty again after the swing ends")
{
	MeleeAttack atk;
	atk.trigger();
	// Advance well past one full attack duration (frames * frameDuration < 10s).
	for (int i = 0; i < 200; ++i)
		atk.update(0.1f);
	REQUIRE_FALSE(atk.isMeleeActive());
	REQUIRE_FALSE(atk.getHitbox({100.f, 100.f}, Direction::Right).has_value());
}

TEST_CASE("MeleeAttack: hitbox sits on the facing side of the player (boundary on x)")
{
	MeleeAttack atk;
	atk.trigger();
	const sf::Vector2f playerPos{100.f, 100.f};

	const auto right = atk.getHitbox(playerPos, Direction::Right);
	const auto left = atk.getHitbox(playerPos, Direction::Left);
	REQUIRE(right.has_value());
	REQUIRE(left.has_value());

	// Right-facing hitbox is to the right of the player center;
	// left-facing hitbox is to the left.
	REQUIRE(right->bounds.position.x >= playerPos.x);
	REQUIRE(left->bounds.position.x + left->bounds.size.x <= playerPos.x);
}

TEST_CASE("MeleeAttack: sourceId is stable across frames within one swing")
{
	MeleeAttack atk;
	atk.trigger();
	const std::uint32_t initial = atk.getSourceId();
	atk.update(0.01f);
	atk.update(0.01f);
	REQUIRE(atk.getSourceId() == initial);
}

TEST_CASE("MeleeAttack: a fresh swing after the previous one ends gets a new sourceId")
{
	MeleeAttack atk;
	atk.trigger();
	const std::uint32_t first = atk.getSourceId();
	for (int i = 0; i < 200; ++i)
		atk.update(0.1f); // ends the swing
	REQUIRE_FALSE(atk.isMeleeActive());

	atk.trigger();
	REQUIRE(atk.getSourceId() != first);
}

TEST_CASE("MeleeAttack: reset() clears the active swing so getHitbox is empty (failure path)")
{
	MeleeAttack atk;
	atk.trigger();
	REQUIRE(atk.isMeleeActive());
	atk.reset();
	REQUIRE_FALSE(atk.isMeleeActive());
	REQUIRE_FALSE(atk.getHitbox({0.f, 0.f}, Direction::Right).has_value());
}
