#include "death_state.h"
#include "../segfault_boss.h"

namespace segfault_boss {

EnemyState *DeathState::update(float deltaTime, BaseEnemy &enemy, const World &world, sf::Vector2f playerPos)
{
	// Boss never leaves this state.
	return this;
}

void DeathState::updateAnimation(float /*deltaTime*/, BaseEnemy &enemy)
{
	auto &boss = static_cast<SegfaultBoss &>(enemy);
	boss.setAnimation(SegfaultBoss::SegfaultBossAnimation::Death, 0);
}

void DeathState::onEnter(BaseEnemy &enemy)
{
	auto &boss = static_cast<SegfaultBoss &>(enemy);
	boss.setVelocityX(0.f);
	boss.setInvincible(true);
}

json DeathState::serialize() const
{
	json j = EnemyState::serialize();
	j["type"] = "SegfaultDeathState";
	return j;
}

} // namespace segfault_boss
