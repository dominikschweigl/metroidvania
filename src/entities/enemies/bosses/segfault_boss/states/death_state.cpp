#include "death_state.h"
#include "../segfault_boss.h"

namespace {
// How long to hold the death pose before triggering the victory screen.
constexpr float kDeathHoldSeconds = 2.0f;
} // namespace

namespace segfault_boss {

EnemyState *DeathState::update(float deltaTime, BaseEnemy &enemy, const World & /*world*/,
                               sf::Vector2f /*playerPos*/)
{
	if (!victorySignaled) {
		frameTimer += deltaTime;
		if (frameTimer >= kDeathHoldSeconds) {
			auto &boss = static_cast<SegfaultBoss &>(enemy);
			boss.requestVictory();
			victorySignaled = true;
		}
	}
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
	frameTimer = 0.f;
	victorySignaled = false;
}

json DeathState::serialize() const
{
	json j = EnemyState::serialize();
	j["type"] = "SegfaultDeathState";
	return j;
}

} // namespace segfault_boss
