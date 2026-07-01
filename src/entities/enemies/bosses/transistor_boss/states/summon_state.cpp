#include "summon_state.h"
#include "../../../../../core/audio_manager.h"
#include "../transistor_boss.h"

namespace transistor_boss {

EnemyState *SummonState::update(float deltaTime, BaseEnemy &enemy, const World & /*world*/, sf::Vector2f /*playerPos*/)
{
	auto &transistor_boss = static_cast<TransistorBoss &>(enemy);

	timer += deltaTime;
	if (timer > TransistorBoss::SUMMON_DUR)
		return &transistor_boss.states.roaming;

	return this;
}

void SummonState::updateAnimation(float deltaTime, BaseEnemy &enemy)
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

void SummonState::onEnter(BaseEnemy &enemy)
{
	auto &transistor_boss = static_cast<TransistorBoss &>(enemy);
	transistor_boss.setVelocityX(0.f);
	transistor_boss.spawnBondedCapacitors();
	currentFrame = 0;
	frameTimer = 0.f;

	AudioManager::getInstance().playSound(SoundEffect::TRANSISTOR_BOSS_CHARGE_ATTACK_WINDUP);
}

void SummonState::onExit(BaseEnemy & /*enemy*/)
{
	timer = 0.f;
}

json SummonState::serialize() const
{
	json j = EnemyState::serialize();

	j["type"] = "SummonState";

	return j;
}
} // namespace transistor_boss
