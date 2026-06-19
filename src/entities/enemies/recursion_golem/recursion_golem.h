#pragma once
#include "../../../combat/hitbox.h"
#include "../base_enemy.h"
#include "states/attack_state.h"
#include "states/chase_state.h"
#include "states/explode_state.h"
#include "states/idle_state.h"
#include "states/windup_state.h"
#include <SFML/Graphics.hpp>
#include <cstdint>
#include <memory>
#include <random>
#include <vector>

// Recursion-themed golem enemy.
// Splits into multiple children after defeat.
// Last split runs a "Stack Overflow" countdown then explodes.
// State flow: Idle -> Chase -> WindUp -> Attack. Explode is entered on base-case defeat.
class RecursionGolem : public BaseEnemy {
  public:
	enum class GolemAnimation { Idle, Moving, WindUp, Attack, Explode };

	static constexpr int DEFAULT_SIZE = 3;
	static constexpr int MAX_SIZE = 4;

	static constexpr float BASE_MOVE_SPEED = 95.f;
	static constexpr float DETECT_RANGE = 420.f;
	static constexpr float ATTACK_RANGE = 34.f;
	static constexpr float LOSE_RANGE = 650.f;

	static constexpr float WINDUP_DUR = 0.4f;
	static constexpr float ATTACK_DUR = 0.45f;
	static constexpr float ATTACK_COOLDOWN = 1.6f;
	static constexpr float ATTACK_LUNGE_SPEED = 130.f;
	static constexpr int ATTACK_DAMAGE = 1;

	static constexpr float BOB_SPEED = 6.f;
	static constexpr float BOB_AMPLITUDE = 1.8f;
	static constexpr float EXPLODE_SWELL = 0.4f;

	static constexpr float EXPLODE_COUNTDOWN = 0.9f;
	static constexpr float EXPLODE_RADIUS_FACTOR = 1.6f;
	static constexpr int EXPLODE_DAMAGE = 2;

	static constexpr float SPLIT_POP_X = 140.f;
	static constexpr float SPLIT_POP_Y = 220.f;

	static constexpr float BASE_WIDTH = 16.f;
	static constexpr float WIDTH_PER_SIZE = 7.f;
	static constexpr int FRAME_SIZE = 32;

	[[nodiscard]] static float widthForSize(int size) noexcept;
	[[nodiscard]] static float heightForSize(int size) noexcept;

	struct States {
		recursion_golem::IdleState idle;
		recursion_golem::ChaseState chase;
		recursion_golem::WindUpState windup;
		recursion_golem::AttackState attack;
		recursion_golem::ExplodeState explode;
	};
	States states;

	RecursionGolem(sf::Vector2f spawnPos, int size);

	void draw(sf::RenderWindow &window) override;

	[[nodiscard]] int getSize() const noexcept { return size_; }
	[[nodiscard]] float moveSpeed() const noexcept;
	[[nodiscard]] bool isBaseCase() const noexcept { return size_ <= 1; }

	[[nodiscard]] bool isAttacking() const noexcept { return currentState == &states.attack; }
	[[nodiscard]] bool isExploding() const noexcept { return isExploding_; }

	[[nodiscard]] std::optional<Hitbox> getHitbox() noexcept override;

	void beginAttackSource() noexcept { attackSourceId = nextSourceId(); }
	[[nodiscard]] std::uint32_t getAttackSourceId() const noexcept { return attackSourceId; }
	void markAttackSourceEnded() noexcept { endedSourceIds.push_back(attackSourceId); }
	void drainEndedSourceIds(std::vector<std::uint32_t> &out) override;

	[[nodiscard]] float getAttackCooldown() const noexcept { return attackCooldown; }
	void setAttackCooldown(float seconds) noexcept { attackCooldown = seconds; }

	void takeDamage(int amount) noexcept override;

	[[nodiscard]] bool isAlive() const noexcept override;

	void drainSpawns(std::vector<std::unique_ptr<BaseEnemy>> &out) override;

	void setAnimation(GolemAnimation anim, int frame);

	json serialize() const override;

  protected:
	void onPreUpdate(float deltaTime) override;

  private:
	const int size_;
	float attackCooldown = 0.f;

	bool defeated_ = false;
	bool resolved_ = false;
	bool removeRequested_ = false;
	bool isExploding_ = false;
	bool explosionFired_ = false;
	bool explosionActive_ = false;
	float explodeTimer_ = 0.f;
	std::uint32_t attackSourceId = 0;
	std::uint32_t explosionSourceId = 0;
	std::vector<std::uint32_t> endedSourceIds;
	std::vector<std::unique_ptr<BaseEnemy>> pendingSpawns_;

	float animClock_ = 0.f;

	const sf::Texture &idleTexture;
	const sf::Texture &movingTexture;
	const sf::Texture &windupTexture;
	const sf::Texture &attackTexture;
	const sf::Texture &explodeTexture;
	sf::Sprite sprite;

	std::mt19937 rng;

	void resolveDefeat();
	void spawnChild(int childSize, float offsetX);
};
