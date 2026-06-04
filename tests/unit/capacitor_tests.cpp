#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "entities/enemies/capacitor/capacitor.h"
#include "world/world.h"

#include "entities/enemies/capacitor/states/flee_state.h"
#include "entities/enemies/capacitor/states/hover_state.h"
#include "entities/enemies/capacitor/states/shoot_state.h"
#include "entities/enemies/capacitor/states/swoop_state.h"

#include <cmath>

namespace {

constexpr float TILE = World::TILE_SIZE;

World makeOpenWorld()
{
	std::vector<std::vector<int>> g(20, std::vector<int>(40, 0));
	for (int x = 0; x < 40; ++x)
		g[18][x] = 1; // a floor far below
	World w;
	w.loadFromGrid(g);
	return w;
}

sf::Vector2f airSpawn()
{
	return {20.f * TILE, 10.f * TILE};
}

} // namespace

TEST_CASE("Capacitor starts hovering, weightless, with no shoot cooldown")
{
	Capacitor c(airSpawn());
	REQUIRE(c.getState() == &c.states.hover);
	REQUIRE(c.getPosition() == airSpawn());
	REQUIRE(c.gravity == 0.f);
	REQUIRE_FALSE(c.isShootOnCooldown());
	REQUIRE(c.shotCount() == 0u);
}

TEST_CASE("Capacitor::hoverTarget keeps the enemy within the player's reach")
{
	Capacitor c(airSpawn());
	const sf::Vector2f player{airSpawn().x + 200.f, airSpawn().y + 50.f};
	const sf::Vector2f target = c.hoverTarget(player);

	SECTION("never steers higher than MAX_REACH_ABOVE over the player's feet")
	{
		REQUIRE(target.y >= player.y - Capacitor::MAX_REACH_ABOVE);
		REQUIRE(target.y == Catch::Approx(player.y - Capacitor::HOVER_HEIGHT));
	}

	SECTION("holds a horizontal standoff on the side it is currently on")
	{
		// Capacitor spawned to the left of the player here.
		REQUIRE(target.x == Catch::Approx(player.x - Capacitor::STANDOFF_X));
	}
}

TEST_CASE("Capacitor HoverState transitions")
{
	World w = makeOpenWorld();
	Capacitor c(airSpawn());

	SECTION("stays hovering and eases velocity when the player is out of range")
	{
		c.setVelocity({100.f, 0.f});
		sf::Vector2f far = c.getPosition() + sf::Vector2f{Capacitor::DETECT_RANGE + 100.f, 0.f};
		REQUIRE(c.states.hover.update(0.016f, c, w, far) == &c.states.hover);
		REQUIRE(std::abs(c.getVelocity().x) < 100.f);
	}

	SECTION("transitions to shoot when in range and the cooldown is ready")
	{
		sf::Vector2f near = c.getPosition() + sf::Vector2f{150.f, 0.f};
		REQUIRE(c.states.hover.update(0.016f, c, w, near) == &c.states.shoot);
	}

	SECTION("keeps hovering while the shoot cooldown is active")
	{
		c.startShootCooldown();
		sf::Vector2f near = c.getPosition() + sf::Vector2f{150.f, 0.f};
		REQUIRE(c.states.hover.update(0.016f, c, w, near) == &c.states.hover);
	}

	SECTION("steers toward the hover target at move speed")
	{
		c.startShootCooldown(); // suppress the transition so we observe steering
		sf::Vector2f player = c.getPosition() + sf::Vector2f{250.f, 0.f};
		(void)c.states.hover.update(0.016f, c, w, player);
		const sf::Vector2f v = c.getVelocity();
		REQUIRE(std::hypot(v.x, v.y) == Catch::Approx(Capacitor::MOVE_SPEED).margin(0.5f));
		REQUIRE(v.x > 0.f); // target is to the right
	}
}

TEST_CASE("Capacitor ShootState fires exactly one shot and then flees")
{
	World w = makeOpenWorld();
	Capacitor c(airSpawn());
	sf::Vector2f player = c.getPosition() + sf::Vector2f{150.f, 0.f};

	c.states.shoot.onEnter(c);
	REQUIRE(c.shotCount() == 0u);

	// First tick fires a single shot and stays in shoot for the brief hold.
	REQUIRE(c.states.shoot.update(0.01f, c, w, player) == &c.states.shoot);
	REQUIRE(c.shotCount() == 1u);

	// Another sub-duration tick does not fire again.
	REQUIRE(c.states.shoot.update(0.01f, c, w, player) == &c.states.shoot);
	REQUIRE(c.shotCount() == 1u);

	// After SHOOT_DUR it retreats (flee) and arms the cooldown on exit.
	REQUIRE(c.states.shoot.update(Capacitor::SHOOT_DUR, c, w, player) == &c.states.flee);
	c.states.shoot.onExit(c);
	REQUIRE(c.isShootOnCooldown());
}

TEST_CASE("Capacitor FleeState retreats away from the player then returns to hover")
{
	World w = makeOpenWorld();
	Capacitor c(airSpawn());
	sf::Vector2f player = c.getPosition() + sf::Vector2f{120.f, 0.f}; // player to the right

	c.states.flee.onEnter(c);

	// Mid-burst: velocity points away from the player (left) and upward.
	REQUIRE(c.states.flee.update(0.1f, c, w, player) == &c.states.flee);
	REQUIRE(c.getVelocity().x < 0.f);
	REQUIRE(c.getVelocity().y < 0.f);

	// After FLEE_DUR it returns to hover.
	REQUIRE(c.states.flee.update(Capacitor::FLEE_DUR, c, w, player) == &c.states.hover);
}

TEST_CASE("Capacitor dives in after several shots so it can be punished")
{
	World w = makeOpenWorld();
	Capacitor c(airSpawn());
	sf::Vector2f player = c.getPosition() + sf::Vector2f{150.f, 0.f};

	// The first few shots end in a flee - still hard to hit.
	for (int i = 0; i < Capacitor::SWOOP_AFTER_SHOTS - 1; ++i)
		c.spawnShot(player);
	REQUIRE(!c.shouldSwoop());

	// The threshold-reaching shot makes the shoot state end in a swoop, not a flee.
	c.states.shoot.onEnter(c);
	(void)c.states.shoot.update(0.01f, c, w, player); // fires the SWOOP_AFTER_SHOTS-th shot
	REQUIRE(c.shouldSwoop());
	REQUIRE(c.states.shoot.update(Capacitor::SHOOT_DUR, c, w, player) == &c.states.swoop);
}

TEST_CASE("Capacitor SwoopState dives close, stays reachable, and resets the counter")
{
	World w = makeOpenWorld();
	Capacitor c(airSpawn());
	sf::Vector2f player = c.getPosition() + sf::Vector2f{200.f, 40.f};

	SECTION("the dive target is closer and lower than the evasive hover target")
	{
		const sf::Vector2f hover = c.hoverTarget(player);
		const sf::Vector2f dive = c.swoopTarget(player);
		REQUIRE(std::abs(dive.x - player.x) < std::abs(hover.x - player.x));
		REQUIRE(dive.y > hover.y); // larger y = lower = nearer the player's feet
	}

	SECTION("entering the swoop clears the shot counter")
	{
		for (int i = 0; i < Capacitor::SWOOP_AFTER_SHOTS; ++i)
			c.spawnShot(player);
		REQUIRE(c.shouldSwoop());
		c.states.swoop.onEnter(c);
		REQUIRE_FALSE(c.shouldSwoop());
	}

	SECTION("dives toward the player, and the safety cap eventually ends it")
	{
		c.states.swoop.onEnter(c);
		REQUIRE(c.states.swoop.update(0.05f, c, w, player) == &c.states.swoop);
		REQUIRE(c.getVelocity().x > 0.f); // homing toward the player on the right
		// Player stays out of reach (direct state ticks don't move the body), so the
		// safety cap is what returns it to flee.
		REQUIRE(c.states.swoop.update(Capacitor::SWOOP_MAX_DUR, c, w, player) == &c.states.flee);
	}
}

TEST_CASE("Capacitor swoop arrives next to the player then lingers before fleeing")
{
	World w = makeOpenWorld();
	sf::Vector2f player{300.f, 300.f};
	// Spawn exactly at the dive target (player's right) so it is already "arrived".
	Capacitor c({player.x + Capacitor::SWOOP_STANDOFF, player.y - Capacitor::SWOOP_HEIGHT});

	c.states.swoop.onEnter(c);

	// Close enough on the first tick -> begins the linger (still swooping).
	REQUIRE(c.states.swoop.update(0.05f, c, w, player) == &c.states.swoop);

	// Once the linger window elapses it peels away.
	REQUIRE(c.states.swoop.update(Capacitor::SWOOP_LINGER, c, w, player) == &c.states.flee);
}

TEST_CASE("Capacitor flee is a short bounded hop that stays on screen")
{
	World w = makeOpenWorld();
	Capacitor c(airSpawn());

	// If it is already past the flee distance, it stops retreating at once.
	sf::Vector2f distantPlayer = c.getPosition() - sf::Vector2f{Capacitor::FLEE_DISTANCE + 40.f, 0.f};
	c.states.flee.onEnter(c);
	REQUIRE(c.states.flee.update(0.01f, c, w, distantPlayer) == &c.states.hover);
}

TEST_CASE("Capacitor energy shots are slower and smaller than the boss's")
{
	projectiles::ElectricBall weak({0.f, 0.f}, {1.f, 0.f}, Capacitor::SHOT_SPEED, Capacitor::SHOT_RADIUS);
	projectiles::ElectricBall boss({0.f, 0.f}, {1.f, 0.f}); // boss defaults

	const float weakX0 = weak.getBounds().position.x;
	const float bossX0 = boss.getBounds().position.x;
	(void)weak.update(0.1f);
	(void)boss.update(0.1f);

	REQUIRE((weak.getBounds().position.x - weakX0) < (boss.getBounds().position.x - bossX0)); // slower
	REQUIRE(weak.getBounds().size.x < boss.getBounds().size.x);                               // smaller
}

TEST_CASE("Capacitor publishes shot hitboxes only once it has fired")
{
	World w = makeOpenWorld();
	Capacitor c(airSpawn());
	sf::Vector2f player = c.getPosition() + sf::Vector2f{150.f, 0.f};

	std::vector<Hitbox> hitboxes;
	c.collectHitboxes(hitboxes);
	REQUIRE(hitboxes.empty());

	c.states.shoot.onEnter(c);
	(void)c.states.shoot.update(0.01f, c, w, player);

	hitboxes.clear();
	c.collectHitboxes(hitboxes);
	REQUIRE(hitboxes.size() == 1u);
	REQUIRE(hitboxes[0].team == Team::Enemy);
	REQUIRE(hitboxes[0].damage == projectiles::ElectricBall::DAMAGE);
}

TEST_CASE("Capacitor is a killable Enemy-team target")
{
	Capacitor c(airSpawn());
	const Hurtbox hurt = c.getHurtbox();
	REQUIRE(hurt.team == Team::Enemy);
	REQUIRE(hurt.health == &c.health);
	REQUIRE_FALSE(hurt.invulnerable);

	REQUIRE(c.isAlive());
	c.takeDamage(Capacitor::CAPACITOR_HEALTH);
	REQUIRE_FALSE(c.isAlive());
}

TEST_CASE("Capacitor end-to-end: hover -> shoot -> flee -> hover")
{
	World w = makeOpenWorld();
	Capacitor c(airSpawn());
	sf::Vector2f player = c.getPosition() + sf::Vector2f{150.f, 0.f};

	// Frame 1: in range + cooldown ready -> hover hands off to shoot. The new
	// state's update runs next frame, so no shot is fired yet.
	c.update(0.016f, w, player, {});
	REQUIRE(c.getState() == &c.states.shoot);

	// Frame 2: shoot fires a single shot.
	c.update(0.016f, w, player, {});
	REQUIRE(c.shotCount() == 1u);

	// Hold elapses -> flee, now on cooldown.
	c.update(Capacitor::SHOOT_DUR, w, player, {});
	REQUIRE(c.getState() == &c.states.flee);
	REQUIRE(c.isShootOnCooldown());

	// Flee burst elapses -> back to hover.
	c.update(Capacitor::FLEE_DUR, w, player, {});
	REQUIRE(c.getState() == &c.states.hover);
}
