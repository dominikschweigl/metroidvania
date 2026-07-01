#include "recover_state.h"
#include "../transistor_boss.h"

namespace transistor_boss {

EnemyState *RecoverState::update(float deltaTime, BaseEnemy &enemy, const World & /*world*/, sf::Vector2f /*playerPos*/)
{

	auto &transistor_boss = static_cast<TransistorBoss &>(enemy);

	recoverTimer += deltaTime;

	if (recoverTimer > TransistorBoss::RECOVER_DUR) {
		return &transistor_boss.states.roaming;
	}

	return this;
}

void RecoverState::updateAnimation(float deltaTime, BaseEnemy &enemy)
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

void RecoverState::onEnter(BaseEnemy &enemy)
{
	auto &transistor_boss = static_cast<TransistorBoss &>(enemy);
	transistor_boss.setVelocityX(0.f);
	transistor_boss.setAuraPhase(TransistorBoss::AuraPhase::None);
	currentFrame = 0;
	frameTimer = 0.f;
}

void RecoverState::onExit(BaseEnemy & /*enemy*/)
{
	recoverTimer = 0.f;
}

json RecoverState::serialize() const
{
	json j = EnemyState::serialize();

	j["type"] = "RecoverState";

	return j;
}

} // namespace transistor_boss
