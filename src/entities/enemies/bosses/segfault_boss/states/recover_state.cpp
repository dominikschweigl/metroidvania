#include "recover_state.h"
#include "../segfault_boss.h"

namespace segfault_boss {

EnemyState *RecoverState::update(float deltaTime, BaseEnemy &enemy, const World &world, sf::Vector2f playerPos)
{
	auto &boss = static_cast<SegfaultBoss &>(enemy);

	timer += deltaTime;
	if (timer > SegfaultBoss::SPEAR_RECOVER_DUR)
		return &boss.states.roaming;

	return this;
}

void RecoverState::updateAnimation(float deltaTime, BaseEnemy &enemy)
{
	auto &boss = static_cast<SegfaultBoss &>(enemy);
	boss.setAnimation(SegfaultBoss::SegfaultBossAnimation::Idle, 0);
}

void RecoverState::onEnter(BaseEnemy &enemy)
{
	auto &boss = static_cast<SegfaultBoss &>(enemy);
	boss.setVelocityX(0.f);
	currentFrame = 0;
	frameTimer = 0.f;
}

void RecoverState::onExit(BaseEnemy & /*enemy*/)
{
	timer = 0.f;
}

json RecoverState::serialize() const
{
	json j = EnemyState::serialize();
	j["type"] = "SegfaultRecoverState";
	return j;
}

} // namespace segfault_boss
