#include "charge_attack_state.h"
#include "../transistor_boss.h"

namespace transistor_boss {

EnemyState *ChargeAttackState::update(float deltaTime, BaseEnemy &enemy, const World &world, sf::Vector2f playerPos)
{

	auto &transistor_boss = static_cast<TransistorBoss &>(enemy);

	damageTimer += deltaTime;

	if (damageTimer > TransistorBoss::CHARGE_DAMAGE_DUR) {
		return &transistor_boss.states.recover;
	}

	return this;
}

void ChargeAttackState::updateAnimation(float deltaTime, BaseEnemy &enemy)
{
	auto &transistor_boss = static_cast<TransistorBoss &>(enemy);

	constexpr int FRAME_COUNT = 2;
	constexpr float FRAME_DURATION = 0.1f;

	frameTimer += deltaTime;
	if (frameTimer >= FRAME_DURATION) {
		frameTimer -= FRAME_DURATION;
		currentFrame = (currentFrame + 1) % FRAME_COUNT;
	}

	transistor_boss.setAnimation(TransistorBoss::TransistorBossAnimation::ChargeAttack, currentFrame);
}

void ChargeAttackState::onEnter(BaseEnemy &enemy)
{
	auto &transistor_boss = static_cast<TransistorBoss &>(enemy);
	transistor_boss.setVelocityX(0.f);
	transistor_boss.setAuraPhase(TransistorBoss::AuraPhase::Damage);
	transistor_boss.armChargeHitbox();
	currentFrame = 0;
	frameTimer = 0.f;
}

void ChargeAttackState::onExit(BaseEnemy &enemy)
{
	auto &transistor_boss = static_cast<TransistorBoss &>(enemy);
	transistor_boss.startChargeCooldown();
	transistor_boss.disarmChargeHitbox();
	damageTimer = 0;
}

} // namespace transistor_boss