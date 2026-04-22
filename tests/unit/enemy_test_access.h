#pragma once

#include "entities/base/base_enemy.h"
#include "entities/base/enemy_state.h"
#include "entities/race_condition_slime/race_condition_slime.h"
#include "world/world.h"

// Test-only access helpers. Their corresponding `friend struct ...;` declarations
// live in the production headers so no game code depends on these types.
struct EnemyTestAccess {
	static void setPos(BaseEnemy &e, sf::Vector2f p) { e.pos = p; }
	static void setVel(BaseEnemy &e, sf::Vector2f v) { e.vel = v; }
	static void setOnGround(BaseEnemy &e, bool g) { e.isOnGround = g; }
	static void setFacing(BaseEnemy &e, BaseEnemy::Direction d) { e.facing = d; }
	static void setCurrentState(BaseEnemy &e, EnemyState *s) { e.currentState = s; }
	static EnemyState *getCurrentState(BaseEnemy &e) { return e.currentState; }

	static float applyGravity(BaseEnemy &e, float dt, const World &w)
	{
		e.applyGravity(dt, w);
		return e.vel.y;
	}
	static bool isGroundBelow(const BaseEnemy &e, const World &w) { return e.isGroundBelow(w); }
	static float resolveHorizontal(BaseEnemy &e, float dt, const World &w) { return e.resolveHorizontal(dt, w); }
	static float resolveVertical(BaseEnemy &e, float dt, const World &w) { return e.resolveVertical(dt, w); }
};

struct SlimeTestAccess {
	static float getJumpCooldown(const RaceConditionSlime &s) { return s.jumpCooldown; }
	static void setJumpCooldown(RaceConditionSlime &s, float v) { s.jumpCooldown = v; }
	static float getTeleportTimer(const RaceConditionSlime &s) { return s.teleportTimer; }
	static void setTeleportTimer(RaceConditionSlime &s, float v) { s.teleportTimer = v; }
};
