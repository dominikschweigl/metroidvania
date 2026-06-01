#include "charge_attack_windup_state.h"
#include "../transistor_boss.h"

namespace transistor_boss {

EnemyState *ChargeAttackWindupState::update(float deltaTime, BaseEnemy &enemy, const World &world,
                                            sf::Vector2f playerPos)
{

	auto &transistor_boss = static_cast<TransistorBoss &>(enemy);

	chargeTimer += deltaTime;

	if (chargeTimer > TransistorBoss::CHARGE_WINDUP_DUR) {
		return &transistor_boss.states.chargeAttack;
	}

	return this;
}

void ChargeAttackWindupState::updateAnimation(float deltaTime, BaseEnemy &enemy)
{
	auto &transistor_boss = static_cast<TransistorBoss &>(enemy);

	constexpr int FRAME_COUNT = 16;
	constexpr float FRAME_DURATION = 0.1f;

	frameTimer += deltaTime;
	if (frameTimer >= FRAME_DURATION) {
		frameTimer -= FRAME_DURATION;
		currentFrame = (currentFrame + 1) % FRAME_COUNT;
	}

	transistor_boss.setAnimation(TransistorBoss::TransistorBossAnimation::ChargeAttackWindup, currentFrame);
}

void ChargeAttackWindupState::onEnter(BaseEnemy &enemy)
{
	auto &transistor_boss = static_cast<TransistorBoss &>(enemy);
	transistor_boss.setVelocityX(0.f);
	transistor_boss.setAuraPhase(TransistorBoss::AuraPhase::Windup);
	currentFrame = 0;
	frameTimer = 0.f;
}

void ChargeAttackWindupState::onExit(BaseEnemy &enemy)
{
	chargeTimer = 0;
}

} // namespace transistor_boss