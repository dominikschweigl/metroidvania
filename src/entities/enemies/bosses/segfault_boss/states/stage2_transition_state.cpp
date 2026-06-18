#include "stage2_transition_state.h"
#include "../segfault_boss.h"

namespace segfault_boss {

EnemyState *Stage2TransitionState::update(float deltaTime, BaseEnemy &enemy, const World &world, sf::Vector2f playerPos)
{
	auto &boss = static_cast<SegfaultBoss &>(enemy);

	timer += deltaTime;
	if (timer > SegfaultBoss::STAGE2_TRANSITION_DUR)
		return &boss.states.summon;

	return this;
}

void Stage2TransitionState::updateAnimation(float deltaTime, BaseEnemy &enemy)
{
	auto &boss = static_cast<SegfaultBoss &>(enemy);
	boss.setAnimation(SegfaultBoss::SegfaultBossAnimation::Idle, 0);
}

void Stage2TransitionState::onEnter(BaseEnemy &enemy)
{
	auto &boss = static_cast<SegfaultBoss &>(enemy);
	boss.setVelocityX(0.f);
	boss.setInvincible(true);
	currentFrame = 0;
	frameTimer = 0.f;
}

void Stage2TransitionState::onExit(BaseEnemy &enemy)
{
	auto &boss = static_cast<SegfaultBoss &>(enemy);
	boss.setInvincible(false);
	timer = 0.f;
}

json Stage2TransitionState::serialize() const
{
	json j = EnemyState::serialize();
	j["type"] = "SegfaultStage2TransitionState";
	return j;
}

} // namespace segfault_boss
