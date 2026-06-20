#include "summon_state.h"
#include "../segfault_boss.h"

namespace segfault_boss {

EnemyState *SummonState::update(float deltaTime, BaseEnemy &enemy, const World &world, sf::Vector2f playerPos)
{
	auto &boss = static_cast<SegfaultBoss &>(enemy);

	timer += deltaTime;
	if (timer > SegfaultBoss::SUMMON_DUR)
		return &boss.states.roaming;

	return this;
}

void SummonState::updateAnimation(float deltaTime, BaseEnemy &enemy)
{
	auto &boss = static_cast<SegfaultBoss &>(enemy);

	constexpr int FRAME_COUNT = 12;
	constexpr float FRAME_DURATION = 0.1f;

	frameTimer += deltaTime;
	if (frameTimer >= FRAME_DURATION) {
		frameTimer -= FRAME_DURATION;
		currentFrame = (currentFrame + 1) % FRAME_COUNT;
	}

	boss.setAnimation(SegfaultBoss::SegfaultBossAnimation::Attack, currentFrame);
}

void SummonState::onEnter(BaseEnemy &enemy)
{
	auto &boss = static_cast<SegfaultBoss &>(enemy);
	boss.setVelocityX(0.f);
	boss.spawnSummonedProcesses();
	currentFrame = 0;
	frameTimer = 0.f;
}

void SummonState::onExit(BaseEnemy & /*enemy*/)
{
	timer = 0.f;
}

json SummonState::serialize() const
{
	json j = EnemyState::serialize();
	j["type"] = "SegfaultSummonState";
	return j;
}

} // namespace segfault_boss
