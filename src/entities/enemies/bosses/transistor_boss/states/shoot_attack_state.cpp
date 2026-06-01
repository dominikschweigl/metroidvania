#include "shoot_attack_state.h"
#include "../transistor_boss.h"

namespace transistor_boss {

EnemyState *ShootAttackState::update(float deltaTime, BaseEnemy &enemy, const World &world, sf::Vector2f playerPos)
{
	auto &transistor_boss = static_cast<TransistorBoss &>(enemy);

	shotTimer += deltaTime;
	if (shotTimer >= TransistorBoss::SHOOT_INTERVAL && shotsFired < TransistorBoss::SHOOT_COUNT) {
		shotTimer = 0.f;
		transistor_boss.spawnElectricBall(playerPos);
		++shotsFired;
	}

	if (shotsFired >= TransistorBoss::SHOOT_COUNT) {
		return &transistor_boss.states.recover;
	}

	return this;
}

void ShootAttackState::updateAnimation(float deltaTime, BaseEnemy &enemy)
{
	auto &transistor_boss = static_cast<TransistorBoss &>(enemy);

	constexpr int FRAME_COUNT = 16;
	constexpr float FRAME_DURATION = 0.1f;

	frameTimer += deltaTime;
	if (frameTimer >= FRAME_DURATION) {
		frameTimer -= FRAME_DURATION;
		currentFrame = (currentFrame + 1) % FRAME_COUNT;
	}

	transistor_boss.setAnimation(TransistorBoss::TransistorBossAnimation::Roaming, currentFrame);
}

void ShootAttackState::onEnter(BaseEnemy &enemy)
{
	auto &transistor_boss = static_cast<TransistorBoss &>(enemy);
	transistor_boss.setVelocityX(0.f);
	shotTimer = TransistorBoss::SHOOT_INTERVAL;
	shotsFired = 0;
	currentFrame = 0;
	frameTimer = 0.f;
}

void ShootAttackState::onExit(BaseEnemy &enemy)
{
	auto &transistor_boss = static_cast<TransistorBoss &>(enemy);
	transistor_boss.startShootCooldown();
}

} // namespace transistor_boss
