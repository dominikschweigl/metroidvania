#include <catch2/catch_test_macros.hpp>

#include "combat/combat_system.h"
#include "combat/hitbox.h"
#include "effects/effect.h"
#include "entities/player/player.h"
#include "world_test_helpers.h"

// --- F4: debug invincibility ---

TEST_CASE("Player: debug invincibility makes isInvulnerable true with zero iframes", "[debug]")
{
	Player p;
	REQUIRE(p.getIframes() == 0.f);
	REQUIRE_FALSE(p.isInvulnerable());

	p.setDebugInvincible(true);
	REQUIRE(p.getIframes() == 0.f); // iframes untouched
	REQUIRE(p.isInvulnerable());
}

TEST_CASE("Player: toggling debug invincibility off restores normal vulnerability", "[debug]")
{
	Player p;
	p.setDebugInvincible(true);
	REQUIRE(p.isInvulnerable());

	p.setDebugInvincible(false);
	REQUIRE(p.getIframes() == 0.f);
	REQUIRE_FALSE(p.isInvulnerable());
}

TEST_CASE("Player: debug invincibility is reflected in the hurtbox", "[debug]")
{
	Player p;
	REQUIRE_FALSE(p.getHurtbox().invulnerable);

	p.setDebugInvincible(true);
	REQUIRE(p.getHurtbox().invulnerable);

	p.setDebugInvincible(false);
	REQUIRE_FALSE(p.getHurtbox().invulnerable);
}

TEST_CASE("Player: debug invincibility is reflected via BaseEntity virtual dispatch", "[debug]")
{
	Player p;
	BaseEntity &asBase = p;

	REQUIRE_FALSE(asBase.isInvulnerable());

	p.setDebugInvincible(true);
	REQUIRE(asBase.isInvulnerable());
	REQUIRE(asBase.getHurtbox().invulnerable);
}

TEST_CASE("Player + CombatSystem: debug-invincible player takes no damage", "[debug]")
{
	Player p;
	p.setDebugInvincible(true);

	const Hitbox enemyHit{p.getBounds(), 1, Team::Enemy, 99};
	std::vector<Hitbox> hits{enemyHit};
	std::vector<Hurtbox> hurts{p.getHurtbox()};

	CombatSystem combat;
	combat.resolve(hits, hurts);

	REQUIRE(p.health.current == Player::MAX_HEALTH);
}

TEST_CASE("Player: debug invincibility persists after iframes expire", "[debug]")
{
	Player p;
	World w = makeEmptyWorld();

	p.health.damage(1);
	p.update(0.016f, w); // arm iframes from real damage
	p.setDebugInvincible(true);

	// Let iframes decay fully
	p.update(10.f, w);
	REQUIRE(p.getIframes() == 0.f);

	// Debug flag should keep the player invulnerable
	REQUIRE(p.isInvulnerable());
	REQUIRE(p.getHurtbox().invulnerable);
}

// --- F5: debug buffs ---

TEST_CASE("Player: all four debug buffs coexist in activeEffects", "[debug]")
{
	Player p;
	p.addEffect(Effect::speed());
	p.addEffect(Effect::damage());
	p.addEffect(Effect::resistance());
	p.addEffect(Effect::jumpBoost());

	REQUIRE(p.activeEffects().size() == 4);
}

TEST_CASE("Player: re-applying all four debug buffs resets durations without adding duplicates", "[debug]")
{
	Player p;
	World w = makeEmptyWorld();
	const float fullDuration = Effect::speed().totalDuration;

	p.addEffect(Effect::speed());
	p.addEffect(Effect::damage());
	p.addEffect(Effect::resistance());
	p.addEffect(Effect::jumpBoost());

	p.update(5.f, w); // tick down all durations
	REQUIRE(p.activeEffects().size() == 4);

	// Simulate the per-frame reapply that the debug toggle performs
	p.addEffect(Effect::speed());
	p.addEffect(Effect::damage());
	p.addEffect(Effect::resistance());
	p.addEffect(Effect::jumpBoost());

	REQUIRE(p.activeEffects().size() == 4); // no duplicates
	for (const Effect &e : p.activeEffects())
		REQUIRE(e.remainingDuration == fullDuration); // all durations reset
}

TEST_CASE("Player: debug buffs keep all effects alive across an otherwise-expiring update", "[debug]")
{
	Player p;
	World w = makeEmptyWorld();
	const float duration = Effect::speed().totalDuration;

	p.addEffect(Effect::speed());
	p.addEffect(Effect::damage());
	p.addEffect(Effect::resistance());
	p.addEffect(Effect::jumpBoost());

	// Tick past full duration so effects would normally expire
	p.update(duration + 1.f, w);
	REQUIRE(p.activeEffects().empty());

	// Re-apply as the debug toggle would on the next frame
	p.addEffect(Effect::speed());
	p.addEffect(Effect::damage());
	p.addEffect(Effect::resistance());
	p.addEffect(Effect::jumpBoost());

	REQUIRE(p.activeEffects().size() == 4);
}
