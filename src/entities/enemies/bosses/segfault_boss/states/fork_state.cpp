#include "fork_state.h"
#include "../segfault_boss.h"

namespace segfault_boss {

EnemyState *ForkState::update(float deltaTime, BaseEnemy &enemy, const World &world, sf::Vector2f playerPos)
{
	auto &boss = static_cast<SegfaultBoss &>(enemy);

	timer += deltaTime;
	if (timer > SegfaultBoss::FORK_DUR)
		return &boss.states.roaming;

	return this;
}

void ForkState::updateAnimation(float deltaTime, BaseEnemy &enemy)
{
	auto &boss = static_cast<SegfaultBoss &>(enemy);

	constexpr int FRAME_COUNT = 8;
	constexpr float FRAME_DURATION = 0.1f;

	frameTimer += deltaTime;
	if (frameTimer >= FRAME_DURATION) {
		frameTimer -= FRAME_DURATION;
		currentFrame = (currentFrame + 1) % FRAME_COUNT;
	}

	boss.setAnimation(SegfaultBoss::SegfaultBossAnimation::Attack, currentFrame);
}

void ForkState::onEnter(BaseEnemy &enemy)
{
	auto &boss = static_cast<SegfaultBoss &>(enemy);
	boss.setVelocityX(0.f);
	boss.spawnFork();
	currentFrame = 0;
	frameTimer = 0.f;
}

void ForkState::onExit(BaseEnemy & /*enemy*/)
{
	timer = 0.f;
}

json ForkState::serialize() const
{
	json j = EnemyState::serialize();
	j["type"] = "SegfaultForkState";
	return j;
}

} // namespace segfault_boss
