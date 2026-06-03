#pragma once

#include "../../../projectiles/electric_ball.h"
#include "../../base_enemy.h"
#include "../../capacitor/capacitor.h"

#include "states/charge_attack_state.h"
#include "states/charge_attack_windup_state.h"
#include "states/recover_state.h"
#include "states/roaming_state.h"
#include "states/shoot_attack_state.h"
#include "states/stage2_recover_state.h"
#include "states/summon_state.h"

#include <memory>
#include <vector>

class TransistorBoss : public BaseEnemy {
  public:
	enum class TransistorBossAnimation { Roaming, ChargeAttackWindup, ChargeAttack, Recover };

	enum class AuraPhase { None, Windup, Damage };

	static constexpr float ENTITY_WIDTH = 120.f;
	static constexpr float ENTITY_HEIGHT = 120.f;
	static constexpr int FRAME_SIZE = 128;

	static constexpr int BOSS_HEALTH = 30;

	static constexpr float MOVE_SPEED = 50.f;

	static constexpr float MOVE_DIRECTION_DUR = 7.f;
	static constexpr float CHARGE_WINDUP_DUR = 3.f;
	static constexpr float CHARGE_DAMAGE_DUR = 1.f;
	static constexpr float RECOVER_DUR = 2.f;

	static constexpr float CHARGE_TIMEOUT = 10.f;

	static constexpr float CHARGE_RANGE = 56.f;
	static constexpr float SHOOT_RANGE = 224.f;

	static constexpr int SHOOT_COUNT = 4;
	static constexpr float SHOOT_INTERVAL = 0.45f;
	static constexpr float SHOOT_TIMEOUT = 5.f;

	static constexpr int CHARGE_DAMAGE = 2;

	static constexpr float CHARGE_AURA_RADIUS = 105.f;

	static constexpr int STAGE2_HP = BOSS_HEALTH / 2;
	static constexpr int CAPACITOR_COUNT = 3;
	static constexpr float STAGE2_RECOVER_DUR = 3.f;
	static constexpr float SUMMON_DUR = 1.5f;
	static constexpr float BEAM_TICK = 0.4f;
	static constexpr int BEAM_DAMAGE = 1;
	static constexpr int BEAM_HITBOX_SAMPLES = 6;
	static constexpr float BEAM_HITBOX_SIZE = 16.f;

	struct States {
		transistor_boss::RoamingState roaming;
		transistor_boss::ShootAttackState shootAttack;
		transistor_boss::ChargeAttackWindupState chargeAttackWindup;
		transistor_boss::ChargeAttackState chargeAttack;
		transistor_boss::RecoverState recover;
		transistor_boss::Stage2RecoverState stage2Recover;
		transistor_boss::SummonState summon;
	};
	States states;

	explicit TransistorBoss(sf::Vector2f spawnPos);

	void draw(sf::RenderWindow &window) override;

	void onHit(const Hitbox &hit) noexcept override;

	void setAnimation(TransistorBossAnimation anim, int frame);

	[[nodiscard]] bool isChargeOnCooldown() const noexcept { return chargeAttackCooldown > 0.f; }
	void startChargeCooldown() noexcept { chargeAttackCooldown = CHARGE_TIMEOUT; }

	[[nodiscard]] bool isShootOnCooldown() const noexcept { return shootAttackCooldown > 0.f; }
	void startShootCooldown() noexcept { shootAttackCooldown = SHOOT_TIMEOUT; }

	void setAuraPhase(AuraPhase phase) noexcept { auraPhase = phase; }

	void armChargeHitbox() noexcept { chargeSourceId = nextSourceId(); }
	void disarmChargeHitbox() { endedBallSourceIds.push_back(chargeSourceId); }

	void spawnElectricBall(sf::Vector2f targetPos);

	[[nodiscard]] std::optional<Hitbox> getHitbox() noexcept override;

	void collectHitboxes(std::vector<Hitbox> &hitboxes) override;
	void collectHurtboxes(std::vector<Hurtbox> &hurtboxes) override;

	void drainEndedSourceIds(std::vector<std::uint32_t> &out) override;

	void spawnBondedCapacitors();
	[[nodiscard]] bool isStage2Triggered() const noexcept { return stage2Triggered; }
	[[nodiscard]] std::size_t bondedCapacitorCount() const noexcept { return bondedCapacitors.size(); }

	void setInvincible(bool value) noexcept { invincible = value; }
	[[nodiscard]] bool isInvulnerable() const noexcept override { return invincible; }

  protected:
	void onPreUpdate(float deltaTime) override;

  private:
	void drawChargeAura(sf::RenderWindow &window) const;
	void drawBeam(sf::RenderWindow &window, sf::Vector2f from, sf::Vector2f to) const;
	void collectBeamHitboxes(std::vector<Hitbox> &hitboxes) const;

	const sf::Texture &roamingTexture;
	const sf::Texture &chargeAttackWindupTexture;
	const sf::Texture &chargeAttackTexture;
	const sf::Texture &recoverTexture;
	sf::Sprite sprite;

	float shootAttackCooldown = 0.f;
	float chargeAttackCooldown = 0.f;

	AuraPhase auraPhase = AuraPhase::None;
	float auraTimer = 0.f;

	std::vector<projectiles::ElectricBall> electricBalls;
	std::vector<std::uint32_t> endedBallSourceIds;
	std::uint32_t chargeSourceId = 0;

	std::vector<std::unique_ptr<Capacitor>> bondedCapacitors;
	bool stage2Triggered = false;
	bool invincible = false;
	float beamTickTimer = 0.f;
	std::uint32_t beamSourceId = 0;
};