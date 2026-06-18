#pragma once

#include "../../base_enemy.h"

#include "segfault_boss_renderer.h"

#include "states/death_state.h"
#include "states/null_spear_attack_state.h"
#include "states/recover_state.h"
#include "states/roaming_state.h"
#include "states/stage2_transition_state.h"
#include "states/summon_state.h"

#include <cstdint>
#include <memory>
#include <vector>

// Second-area boss: a corrupted scientist whose process is leaking memory.
class SegfaultBoss : public BaseEnemy {
  public:
	using SegfaultBossAnimation = segfault_boss::SegfaultBossRenderer::Animation;

	enum class SpearPhase { Windup, Strike };

	struct NullSpear {
		sf::Vector2f foot;
		SpearPhase phase = SpearPhase::Windup;
		float timer = 0.f;
		std::uint32_t sourceId = 0;
	};

	static constexpr float ENTITY_WIDTH = 40.f;
	static constexpr float ENTITY_HEIGHT = 64.f;

	static constexpr int BOSS_HEALTH = 36;

	static constexpr float MOVE_SPEED = 70.f;
	static constexpr float MOVE_DIRECTION_DUR = 5.f;

	// NULL spear attack parameters.
	static constexpr float SPEAR_RANGE = 320.f;
	static constexpr int SPEAR_MIN_COUNT = 4;
	static constexpr int SPEAR_MAX_COUNT = 6;
	static constexpr float SPEAR_SPAWN_INTERVAL = 0.5f;
	static constexpr float SPEAR_SPREAD = 240.f;
	static constexpr float SPEAR_WINDUP_DUR = 1.f;
	static constexpr float SPEAR_STRIKE_DUR = 0.45f;
	static constexpr float SPEAR_RECOVER_DUR = 0.8f;
	static constexpr float SPEAR_COOLDOWN = 3.f;
	static constexpr int SPEAR_DAMAGE = 2;
	static constexpr float SPEAR_WIDTH = 26.f;
	static constexpr float SPEAR_HEIGHT = 104.f;

	// Stage thresholds and stage-two summon parameters.
	static constexpr int STAGE2_HP = 24;
	static constexpr int STAGE3_HP = 12;
	static constexpr float STAGE2_TRANSITION_DUR = 2.f;
	static constexpr float SUMMON_DUR = 1.5f;
	static constexpr int SUMMON_COUNT = 3;
	static constexpr float SUMMON_SPREAD = 120.f;
	static constexpr float SUMMON_AIR_HEIGHT = 80.f;

	struct States {
		segfault_boss::RoamingState roaming;
		segfault_boss::NullSpearAttackState nullSpearAttack;
		segfault_boss::RecoverState recover;
		segfault_boss::Stage2TransitionState stage2Transition;
		segfault_boss::SummonState summon;
		segfault_boss::DeathState death;
	};
	States states;

	explicit SegfaultBoss(sf::Vector2f spawnPos);

	void draw(sf::RenderWindow &window) override;

	void onHit(const Hitbox &hit) noexcept override;

	void setAnimation(SegfaultBossAnimation anim, int frame);

	void setInvincible(bool value) noexcept { invincible = value; }
	[[nodiscard]] bool isInvulnerable() const noexcept override { return invincible; }

	[[nodiscard]] bool isSpearOnCooldown() const noexcept { return spearCooldown > 0.f; }
	void startSpearCooldown() noexcept { spearCooldown = SPEAR_COOLDOWN; }

	void spawnNullSpear(float worldX, float fromY, const World &world);

	void spawnNullSpearOnPlayer();

	[[nodiscard]] bool hasActiveSpears() const noexcept { return !spears.empty(); }
	[[nodiscard]] const std::vector<NullSpear> &getSpears() const noexcept { return spears; }

	[[nodiscard]] int getStage() const noexcept { return stage; }
	[[nodiscard]] bool isStage2Triggered() const noexcept { return stage2Triggered; }

	// Spawns the stage-two minions as boss-owned sub-entities.
	void spawnSummonedProcesses();
	[[nodiscard]] std::size_t summonedProcessCount() const noexcept { return summonedProcesses.size(); }

	void collectHitboxes(std::vector<Hitbox> &hitboxes) override;
	void collectHurtboxes(std::vector<Hurtbox> &hurtboxes) override;
	void drainEndedSourceIds(std::vector<std::uint32_t> &out) override;

	// Ability to request a "bluescreen" between stages 2 and 3.
	[[nodiscard]] bool consumeBluescreenRequest() noexcept
	{
		const bool requested = bluescreenRequested;
		bluescreenRequested = false;
		return requested;
	}

	[[nodiscard]] bool isAlive() const noexcept override { return health.isAlive() || dying; }

	json serialize() const override;
	void deserialize(const json &j) override;

  protected:
	void onPreUpdate(float deltaTime) override;

  private:
	segfault_boss::SegfaultBossRenderer renderer;

	bool invincible = false;
	bool dying = false;
	// Raised when crossing into stage three so the scene can show the bluescreen.
	bool bluescreenRequested = false;
	int stage = 1;
	bool stage2Triggered = false;
	Direction deathFacing = Direction::Right;

	std::vector<NullSpear> spears;
	float spearCooldown = 0.f;
	std::vector<std::uint32_t> endedSourceIds;

	std::vector<std::unique_ptr<BaseEnemy>> summonedProcesses;

	float effectTimer = 0.f;
};
