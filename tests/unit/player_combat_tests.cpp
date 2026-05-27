#include <catch2/catch_test_macros.hpp>

#include "combat/combat_system.h"
#include "combat/hitbox.h"
#include "entities/player/player.h"
#include "world/world.h"

namespace {

World makeEmptyWorld()
{
	std::vector<std::vector<int>> g(5, std::vector<int>(10, 0));
	World w;
	w.loadFromGrid(g);
	return w;
}

} // namespace

TEST_CASE("Player: starts with full health (5/5) and is alive")
{
	Player p;
	REQUIRE(p.health.max == 5);
	REQUIRE(p.health.current == 5);
	REQUIRE(p.isAlive());
}

TEST_CASE("Player: starts with no i-frames and vulnerable hurtbox")
{
	Player p;
	REQUIRE(p.getIframes() == 0.f);
	const Hurtbox hurtbox = p.getHurtbox();
	REQUIRE(hurtbox.team == Team::Player);
	REQUIRE(hurtbox.health == &p.health);
	REQUIRE_FALSE(hurtbox.invulnerable);
}

TEST_CASE("Player: taking damage triggers i-frames on next update")
{
	Player p;
	World w = makeEmptyWorld();

	// Simulate damage applied by the CombatSystem between frames.
	p.health.damage(1);
	REQUIRE(p.health.current == 4);

	p.update(0.016f, w);

	REQUIRE(p.getIframes() > 0.f);
	REQUIRE(p.getHurtbox().invulnerable);
}

TEST_CASE("Player: i-frames decay over time and clamp at zero (boundary)")
{
	Player p;
	World w = makeEmptyWorld();

	p.health.damage(1);
	p.update(0.016f, w); // arms i-frames

	const float armed = p.getIframes();
	REQUIRE(armed > 0.f);

	// Tick well past the duration — should clamp at exactly 0, not go negative.
	p.update(10.f, w);
	REQUIRE(p.getIframes() == 0.f);
	REQUIRE_FALSE(p.getHurtbox().invulnerable);
}

TEST_CASE("Player: repeated damage during i-frames does not double-up the timer")
{
	Player p;
	World w = makeEmptyWorld();

	p.health.damage(1);
	p.update(0.016f, w);
	const float firstArm = p.getIframes();

	// Damage applied again while still invulnerable — re-arms to full duration.
	p.health.damage(1);
	p.update(0.016f, w);
	const float secondArm = p.getIframes();

	REQUIRE(secondArm >= firstArm - 0.001f);
	REQUIRE(secondArm <= Player::IFRAME_DURATION);
}

TEST_CASE("Player: dies when health reaches zero (boundary)")
{
	Player p;
	p.health.damage(p.health.current);
	REQUIRE(p.health.current == 0);
	REQUIRE_FALSE(p.isAlive());
}

TEST_CASE("Player: zero damage does not arm i-frames (failure case)")
{
	Player p;
	World w = makeEmptyWorld();

	p.update(0.016f, w);

	REQUIRE(p.getIframes() == 0.f);
	REQUIRE_FALSE(p.getHurtbox().invulnerable);
}

TEST_CASE("Player: hurtbox bounds follow the player's body bounds")
{
	Player p;
	const sf::FloatRect body = p.getBounds();
	const Hurtbox hurtbox = p.getHurtbox();
	REQUIRE(hurtbox.bounds.position == body.position);
	REQUIRE(hurtbox.bounds.size == body.size);
}

TEST_CASE("Player + CombatSystem: enemy hitbox damages player once, then i-frames block follow-ups")
{
	Player p;
	World w = makeEmptyWorld();

	// Build a hitbox that overlaps the player.
	const sf::FloatRect body = p.getBounds();
	const Hitbox enemyHit{body, 1, Team::Enemy, 99};

	CombatSystem combat;

	// Frame 1: vulnerable: takes damage.
	std::vector<Hitbox> hits{enemyHit};
	std::vector<Hurtbox> hurts{p.getHurtbox()};
	combat.resolve(hits, hurts);
	REQUIRE(p.health.current == 4);

	// Player::update sees the HP drop, arms i-frames.
	p.update(0.016f, w);
	REQUIRE(p.getHurtbox().invulnerable);

	// Frame 2: invulnerable: no further damage even with the same source still present.
	hurts = {p.getHurtbox()};
	combat.resolve(hits, hurts);
	REQUIRE(p.health.current == 4);
}
