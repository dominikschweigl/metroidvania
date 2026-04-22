#pragma once
#include "../base/base_enemy.h"
#include "states/attack_state.h"
#include "states/chase_state.h"
#include "states/idle_state.h"
#include "states/recover_state.h"
#include "states/windup_state.h"
#include <SFML/Graphics.hpp>
#include <random>

// Race-condition themed enemy. State flow: Idle -> Chase -> WindUp -> Attack -> Recover.
class RaceConditionSlime : public BaseEnemy {
  public:
	// Motion
	static constexpr float MOVE_SPEED = 150.f;
	static constexpr float MAX_JUMP_SPEED = 700.f;

	// Awareness / combat ranges
	static constexpr float DETECT_RANGE = 420.f;
	static constexpr float ATTACK_RANGE = 28.f;
	static constexpr float LOSE_RANGE = 650.f;

	// Attack phase durations
	static constexpr float WINDUP_DUR = 0.4f;
	static constexpr float ATTACK_DUR = 1.0f;
	static constexpr float RECOVER_DUR = 0.5f;

	// Cooldowns / thresholds
	static constexpr float JUMP_THRESHOLD = 40.f; // player must be at least this much higher
	static constexpr float JUMP_COOLDOWN = 1.2f;
	static constexpr float ATTACK_COOLDOWN = 5.f;

	// Geometry
	static constexpr float ENTITY_WIDTH = 28.f;
	static constexpr float ENTITY_HEIGHT = 28.f;
	static constexpr int FRAME_SIZE = 32;

	// State pool
	struct States {
		rc_slime::IdleState idle;
		rc_slime::ChaseState chase;
		rc_slime::WindUpState windup;
		rc_slime::AttackState attack;
		rc_slime::RecoverState recover;
	};
	States states;

	explicit RaceConditionSlime(sf::Vector2f spawnPos);

	void draw(sf::RenderWindow &window) override;

	// True while mid-strike — used by the player's collision/damage code.
	bool isAttacking() const { return currentState == &states.attack; }

	float getAttackCooldown() const { return attackCooldown; }
	void setAttackCooldown(float t) { attackCooldown = t; }

	// Slime abilities exposed to ChaseState. Kept on the slime (not the state)
	// because they rely on slime-owned data (rng, timers).
	void tryJumpTowards(float heightDiff);
	void maybeTeleport(const World &world, sf::Vector2f playerPos);

  protected:
	// Hook called each frame before state update. Ticks internal timers.
	void onPreUpdate(float deltaTime) override;

  private:
	float attackCooldown = 0.f;
	float jumpCooldown = 0.f;
	float teleportTimer = 0.f;

	sf::Texture idleTexture;
	sf::Texture movingTexture;
	sf::Texture windupTexture;
	sf::Texture attackTexture;
	sf::Texture recoverTexture;
	sf::Sprite sprite{idleTexture};

	std::mt19937 rng;

	// Textures and sprite are private; states access them via friend.
	friend class rc_slime::IdleState;
	friend class rc_slime::ChaseState;
	friend class rc_slime::WindUpState;
	friend class rc_slime::AttackState;
	friend class rc_slime::RecoverState;

	friend struct SlimeTestAccess;

	float uniformFloat(float lo, float hi) { return std::uniform_real_distribution<float>(lo, hi)(rng); }
	void resetTeleportTimer();
	void glitchTeleport(const World &world, sf::Vector2f playerPos);
	bool isValidTeleportDest(const World &world, float newX, float newY) const;
	void loadAssets();
};
