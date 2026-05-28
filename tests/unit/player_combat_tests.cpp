#include <catch2/catch_test_macros.hpp>

#include "combat/combat_system.h"
#include "combat/hitbox.h"
#include "entities/base_entity.h"
#include "entities/player/player.h"
#include "world/world.h"
#include <type_traits>

static_assert(std::is_base_of_v<BaseEntity, Player>, "Player must inherit from BaseEntity");

namespace {

World makeEmptyWorld()
{
	std::vector<std::vector<int>> g(5, std::vector<int>(10, 0));
	World w;
	w.loadFromGrid(g);
	return w;
}

World makeFlooredWorldForPlayer()
{
	// Player spawns at (15*32, 0). Build a wide world with a floor row directly beneath
	// so the player stays in idle (canAttack=true) after the first update tick.
	std::vector<std::vector<int>> g(20, std::vector<int>(40, 0));
	for (int x = 0; x < 40; ++x)
		g[1][x] = 1;
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

	// Tick well past the duration - should clamp at exactly 0, not go negative.
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

	// Damage applied again while still invulnerable - re-arms to full duration.
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

TEST_CASE("Player: setPosition then getPosition round-trips")
{
	Player p;
	const sf::Vector2f target{321.5f, 654.25f};
	p.setPosition(target);
	REQUIRE(p.getPosition() == target);
}

TEST_CASE("Player: getBounds is a 32x32 rectangle anchored at the feet")
{
	Player p;
	p.setPosition({400.f, 200.f});
	const sf::FloatRect b = p.getBounds();
	REQUIRE(b.size.x == 32.f);
	REQUIRE(b.size.y == 32.f);
	REQUIRE(b.position.x == 400.f - 16.f); // pos.x - FRAME_SIZE/2
	REQUIRE(b.position.y == 200.f - 32.f); // pos.y - FRAME_SIZE (foot-anchored)
}

TEST_CASE("Player: default direction is Left and default state is grounded")
{
	Player p;
	REQUIRE(p.getDirection() == Direction::Left);
	REQUIRE(p.isPlayerOnGround());
}

TEST_CASE("Player: hurtbox bounds follow the player's body bounds")
{
	Player p;
	const sf::FloatRect body = p.getBounds();
	const Hurtbox hurtbox = p.getHurtbox();
	REQUIRE(hurtbox.bounds.position == body.position);
	REQUIRE(hurtbox.bounds.size == body.size);
}

TEST_CASE("Player::collectHitboxes (virtual override) appends the active melee hitbox")
{
	Player p;
	std::vector<Hitbox> hitboxes;

	// No swing -> empty.
	p.collectHitboxes(hitboxes);
	REQUIRE(hitboxes.empty());

	// Stay grounded in Idle so canAttack() is true when the attack trigger lands.
	World w = makeFlooredWorldForPlayer();
	p.setPosition({15 * 32.f, 32.f}); // feet sitting on top of the floor row
	p.update(0.016f, w, /*attack=*/true);

	// Drive Player through its base-class interface so we exercise virtual dispatch.
	BaseEntity &asBase = p;
	asBase.collectHitboxes(hitboxes);
	REQUIRE(hitboxes.size() == 1);
	REQUIRE(hitboxes.front().team == Team::Player);
}

TEST_CASE("Player::isInvulnerable (virtual override) drives getHurtbox through the base")
{
	Player p;
	World w = makeEmptyWorld();
	BaseEntity &asBase = p;

	REQUIRE_FALSE(asBase.isInvulnerable());
	REQUIRE_FALSE(asBase.getHurtbox().invulnerable);

	p.health.damage(1);
	p.update(0.016f, w);
	REQUIRE(asBase.isInvulnerable());
	REQUIRE(asBase.getHurtbox().invulnerable);
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
