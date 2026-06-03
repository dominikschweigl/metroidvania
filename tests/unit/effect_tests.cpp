#include <catch2/catch_test_macros.hpp>

#include "combat/hitbox.h"
#include "effects/effect.h"
#include "entities/player/player.h"
#include "world_test_helpers.h"
#include <vector>

// --- Effect struct invariants ---

TEST_CASE("Effect: factory methods produce distinct, non-empty effect IDs", "[effect]")
{
	REQUIRE_FALSE(Effect::jumpBoost().effectId().empty());
	REQUIRE_FALSE(Effect::speed().effectId().empty());
	REQUIRE_FALSE(Effect::damage().effectId().empty());
	REQUIRE_FALSE(Effect::resistance().effectId().empty());

	const std::string_view ids[4] = {
	    Effect::jumpBoost().effectId(),
	    Effect::speed().effectId(),
	    Effect::damage().effectId(),
	    Effect::resistance().effectId(),
	};
	for (int i = 0; i < 4; ++i)
		for (int j = i + 1; j < 4; ++j)
			REQUIRE(ids[i] != ids[j]);
}

TEST_CASE("Effect: remainingDuration equals totalDuration on construction", "[effect]")
{
	SECTION("jumpBoost")
	{
		const Effect e = Effect::jumpBoost();
		REQUIRE(e.remainingDuration == e.totalDuration);
	}
	SECTION("speed")
	{
		const Effect e = Effect::speed();
		REQUIRE(e.remainingDuration == e.totalDuration);
	}
	SECTION("damage")
	{
		const Effect e = Effect::damage();
		REQUIRE(e.remainingDuration == e.totalDuration);
	}
	SECTION("resistance")
	{
		const Effect e = Effect::resistance();
		REQUIRE(e.remainingDuration == e.totalDuration);
	}
}

// --- Player effect management ---

TEST_CASE("Player: no active effects on construction", "[effect]")
{
	Player p;
	REQUIRE(p.activeEffects().empty());
}

TEST_CASE("Player::addEffect stores one effect", "[effect]")
{
	Player p;
	p.addEffect(Effect::speed());
	REQUIRE(p.activeEffects().size() == 1);
}

TEST_CASE("Player::addEffect: same type twice resets duration and does not add a duplicate", "[effect]")
{
	Player p;
	World w = makeEmptyWorld();
	const float fullDuration = Effect::speed().totalDuration;

	p.addEffect(Effect::speed());
	p.update(5.f, w); // tick the duration down
	REQUIRE(p.activeEffects().size() == 1);
	REQUIRE(p.activeEffects().front().remainingDuration < fullDuration);

	p.addEffect(Effect::speed()); // re-add same ID
	REQUIRE(p.activeEffects().size() == 1);
	REQUIRE(p.activeEffects().front().remainingDuration == fullDuration);
}

TEST_CASE("Player::addEffect: two different effect types are stored independently", "[effect]")
{
	Player p;
	p.addEffect(Effect::speed());
	p.addEffect(Effect::jumpBoost());
	REQUIRE(p.activeEffects().size() == 2);
}

// --- Effect expiry ---

TEST_CASE("Player: effect expires when its full duration elapses", "[effect]")
{
	Player p;
	World w = makeEmptyWorld();
	const float duration = Effect::speed().totalDuration;

	p.addEffect(Effect::speed());
	p.update(duration + 0.1f, w);
	REQUIRE(p.activeEffects().empty());
}

TEST_CASE("Player: effect persists while duration has not fully elapsed", "[effect]")
{
	Player p;
	World w = makeEmptyWorld();
	const float duration = Effect::speed().totalDuration;

	p.addEffect(Effect::speed());
	p.update(duration * 0.5f, w);
	REQUIRE(p.activeEffects().size() == 1);
}

// --- Resistance via takeDamage ---

TEST_CASE("Player::takeDamage with resistance reduces incoming damage", "[effect]")
{
	// TODO: fractional resistance path (where amount * (1 - resistance) is not an integer) requires seeded RNG
	Player p;
	p.addEffect(Effect::resistance());
	const int before = p.health.current;
	p.takeDamage(4);
	REQUIRE(p.health.current > before - 4); // less damage taken than the raw amount
}

TEST_CASE("Player::takeDamage: re-adding resistance does not stack the reduction", "[effect]")
{
	const int damage = 4;

	Player withOne;
	withOne.addEffect(Effect::resistance());
	withOne.takeDamage(damage);

	Player withTwo;
	withTwo.addEffect(Effect::resistance());
	withTwo.addEffect(Effect::resistance()); // same ID — resets duration, no second instance
	withTwo.takeDamage(damage);

	// Both should lose the same amount of health
	REQUIRE(withOne.health.current == withTwo.health.current);
}

// --- Damage multiplier via collectHitboxes ---

TEST_CASE("Player: damage effect amplifies melee hitbox damage", "[effect]")
{
	World w = makeFlooredWorldForPlayer();

	Player base;
	base.setPosition({15 * 32.f, 32.f});
	base.update(0.016f, w, /*attackTriggered=*/true);
	std::vector<Hitbox> baseHitboxes;
	base.collectHitboxes(baseHitboxes);
	REQUIRE(baseHitboxes.size() == 1);
	const int baseDamage = baseHitboxes.front().damage;

	Player boosted;
	boosted.setPosition({15 * 32.f, 32.f});
	boosted.addEffect(Effect::damage());
	boosted.update(0.016f, w, /*attackTriggered=*/true);
	std::vector<Hitbox> boostedHitboxes;
	boosted.collectHitboxes(boostedHitboxes);
	REQUIRE(boostedHitboxes.size() == 1);

	REQUIRE(boostedHitboxes.front().damage > baseDamage);
}
