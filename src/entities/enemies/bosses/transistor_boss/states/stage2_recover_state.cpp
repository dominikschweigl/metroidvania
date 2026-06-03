#include "stage2_recover_state.h"
#include "../transistor_boss.h"

namespace transistor_boss {

EnemyState *Stage2RecoverState::update(float deltaTime, BaseEnemy &enemy, const World &world, sf::Vector2f playerPos)
{
	auto &transistor_boss = static_cast<TransistorBoss &>(enemy);

	timer += deltaTime;
	if (timer > TransistorBoss::STAGE2_RECOVER_DUR)
		return &transistor_boss.states.summon;

	return this;
}

void Stage2RecoverState::updateAnimation(float deltaTime, BaseEnemy &enemy)
{
	auto &transistor_boss = static_cast<TransistorBoss &>(enemy);

	constexpr int FRAME_COUNT = 17;
	constexpr float FRAME_DURATION = 0.1f;

	frameTimer += deltaTime;
	if (frameTimer >= FRAME_DURATION) {
		frameTimer -= FRAME_DURATION;
		currentFrame = (currentFrame + 1) % FRAME_COUNT;
	}

	transistor_boss.setAnimation(TransistorBoss::TransistorBossAnimation::Recover, currentFrame);
}

void Stage2RecoverState::onEnter(BaseEnemy &enemy)
{
	auto &transistor_boss = static_cast<TransistorBoss &>(enemy);
	transistor_boss.setVelocityX(0.f);
	transistor_boss.setInvincible(true);
	currentFrame = 0;
	frameTimer = 0.f;
}

void Stage2RecoverState::onExit(BaseEnemy &enemy)
{
	auto &transistor_boss = static_cast<TransistorBoss &>(enemy);
	transistor_boss.setInvincible(false);
	timer = 0.f;
}

} // namespace transistor_boss
