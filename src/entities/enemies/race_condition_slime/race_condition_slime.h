#pragma once
#include "../../../combat/hitbox.h"
#include "../base_enemy.h"
#include "states/attack_state.h"
#include "states/chase_state.h"
#include "states/idle_state.h"
#include "states/recover_state.h"
#include "states/windup_state.h"
#include <SFML/Graphics.hpp>
#include <cstdint>
#include <random>
#include <vector>

// Race-condition themed enemy. State flow: Idle -> Chase -> WindUp -> Attack -> Recover.
class RaceConditionSlime : public BaseEnemy {
  public:
	enum class SlimeAnimation { Idle, Moving, WindUp, Attack, Recover };

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
	static constexpr float MOVE_SOUND_INTERVAL = 0.85f;
	static constexpr float SLIME_VOLUME = 20.f;

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

	explicit RaceConditionSlime(sf::Vector2f spawnPos, float drop_chance);

	void draw(sf::RenderWindow &window) override;

	// True while mid-strike - used by the player's collision/damage code.
	bool isAttacking() const { return currentState == &states.attack; }

	static constexpr int ATTACK_DAMAGE = 1;

	// Active damage rectangle while the slime is mid-strike, otherwise nullopt.
	[[nodiscard]] std::optional<Hitbox> getHitbox() noexcept override
	{
		if (!isAttacking())
			return std::nullopt;
		return Hitbox{getBounds(), ATTACK_DAMAGE, Team::Enemy, attackSourceId};
	}

	void beginAttackSource() noexcept { attackSourceId = nextSourceId(); }
	[[nodiscard]] std::uint32_t getAttackSourceId() const noexcept { return attackSourceId; }

	// Called by AttackState::onExit so the CombatSystem can prune the swing's
	// (sourceId, victim) entries after it stops publishing a hitbox.
	void markAttackSourceEnded() noexcept { endedSourceIds.push_back(attackSourceId); }

	void drainEndedSourceIds(std::vector<std::uint32_t> &out) override;

	float getAttackCooldown() const { return attackCooldown; }
	void setAttackCooldown(float t) { attackCooldown = t; }
	float getJumpCooldown() const { return jumpCooldown; }
	void setJumpCooldown(float v) { jumpCooldown = v; }
	float getTeleportTimer() const { return teleportTimer; }
	void setTeleportTimer(float v) { teleportTimer = v; }

	// Set current animation state. Manages sprite and texture updates.
	void setAnimation(SlimeAnimation anim, int frame);

	// Slime abilities exposed to ChaseState. Kept on the slime (not the state)
	// because they rely on slime-owned data (rng, timers).
	void tryJumpTowards(float heightDiff);
	void maybeTeleport(const World &world, sf::Vector2f playerPos);

	json serialize() const override;

  protected:
	// Hook called each frame before state update. Ticks internal timers.
	void onPreUpdate(float deltaTime) override;

  private:
	float attackCooldown = 0.f;
	float jumpCooldown = 0.f;
	float teleportTimer = 0.f;
	float moveSoundTimer = 0.f;
	std::uint32_t attackSourceId = 0;
	std::vector<std::uint32_t> endedSourceIds;

	const sf::Texture &idleTexture;
	const sf::Texture &movingTexture;
	const sf::Texture &windupTexture;
	const sf::Texture &attackTexture;
	const sf::Texture &recoverTexture;
	sf::Sprite sprite;

	float uniformFloat(float lo, float hi) { return std::uniform_real_distribution<float>(lo, hi)(rng); }
	void resetTeleportTimer();
	void glitchTeleport(const World &world, sf::Vector2f playerPos);
	bool isValidTeleportDest(const World &world, float newX, float newY) const;
};
