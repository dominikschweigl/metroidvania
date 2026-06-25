#pragma once
#include "../../../combat/hitbox.h"
#include "../base_enemy.h"
#include "states/chase_state.h"
#include "states/idle_state.h"
#include "states/jump_attack_state.h"
#include "states/recover_state.h"
#include <SFML/Graphics.hpp>
#include <cstdint>
#include <vector>

// Hardware-resistor themed jumping bug.
// State flow: Idle -> Chase -> JumpAttack -> Recover.
class ResistorBug : public BaseEnemy {
  public:
	enum class BugAnimation { Idle, Moving, Telegraph, Attack, Recover };

	// Motion
	static constexpr float MOVE_SPEED = 90.f;
	static constexpr float HOP_VX = 220.f;
	static constexpr float HOP_VY = 520.f;

	// Awareness / combat ranges
	static constexpr float DETECT_RANGE = 360.f;
	static constexpr float ATTACK_RANGE = 90.f;
	static constexpr float LOSE_RANGE = 560.f;

	// Attack phase durations
	static constexpr float TELEGRAPH_DUR = 0.35f;
	static constexpr float JUMPATTACK_MAX_DUR = 2.0f;
	static constexpr float RECOVER_DUR = 0.45f;

	// Cooldowns
	static constexpr float JUMP_COOLDOWN = 0.4f;
	static constexpr float ATTACK_COOLDOWN = 1.5f;

	// Geometry
	static constexpr float ENTITY_WIDTH = 28.f;
	static constexpr float ENTITY_HEIGHT = 28.f;
	static constexpr int FRAME_SIZE = 32;

	static constexpr int ATTACK_DAMAGE = 1;

	// State pool
	struct States {
		resistor_bug::IdleState idle;
		resistor_bug::ChaseState chase;
		resistor_bug::JumpAttackState jumpAttack;
		resistor_bug::RecoverState recover;
	};
	States states;

	explicit ResistorBug(sf::Vector2f spawnPos);
	explicit ResistorBug(sf::Vector2f spawnPos, float drop_chance);

	void draw(sf::RenderWindow &window) override;

	bool isAttacking() const { return attackActive; }

	[[nodiscard]] std::optional<Hitbox> getHitbox() noexcept override
	{
		if (!isAttacking())
			return std::nullopt;
		return Hitbox{getBounds(), ATTACK_DAMAGE, Team::Enemy, attackSourceId};
	}

	[[nodiscard]] std::uint32_t getAttackSourceId() const noexcept { return attackSourceId; }

	void drainEndedSourceIds(std::vector<std::uint32_t> &out) override;

	float getAttackCooldown() const { return attackCooldown; }
	void setAttackCooldown(float t) { attackCooldown = t; }
	float getJumpCooldown() const { return jumpCooldown; }
	void setJumpCooldown(float v) { jumpCooldown = v; }

	// Set current animation state. Manages sprite and texture updates.
	void setAnimation(BugAnimation anim, int frame);

	// Launches the airborne strike toward the player and opens the damage window.
	// Called by JumpAttackState once the telegraph completes.
	void launchHop(sf::Vector2f playerPos);
	// Closes the damage window. Called by JumpAttackState::onExit.
	void endAttack() noexcept;

	json serialize() const override;

  protected:
	// Hook called each frame before state update. Ticks internal timers.
	void onPreUpdate(float deltaTime) override;

  private:
	float attackCooldown = 0.f;
	float jumpCooldown = 0.f;
	bool attackActive = false;
	std::uint32_t attackSourceId = 0;
	std::vector<std::uint32_t> endedSourceIds;

	const sf::Texture &idleTexture;
	const sf::Texture &movingTexture;
	const sf::Texture &telegraphTexture;
	const sf::Texture &attackTexture;
	const sf::Texture &recoverTexture;
	sf::Sprite sprite;
};
