#include "stage3_transition_state.h"
#include "../segfault_boss.h"

namespace segfault_boss {

EnemyState *Stage3TransitionState::update(float deltaTime, BaseEnemy &enemy, const World & /*world*/,
                                          sf::Vector2f /*playerPos*/)
{
	auto &boss = static_cast<SegfaultBoss &>(enemy);

	timer += deltaTime;
	if (timer > SegfaultBoss::STAGE3_TRANSITION_DUR)
		return &boss.states.fork;

	return this;
}

void Stage3TransitionState::updateAnimation(float deltaTime, BaseEnemy &enemy)
{
	auto &boss = static_cast<SegfaultBoss &>(enemy);

	constexpr int FRAME_COUNT = 10;
	constexpr float FRAME_DURATION = 0.1f;

	frameTimer += deltaTime;
	if (frameTimer >= FRAME_DURATION) {
		frameTimer -= FRAME_DURATION;
		currentFrame = (currentFrame + 1) % FRAME_COUNT;
	}

	boss.setAnimation(SegfaultBoss::SegfaultBossAnimation::Idle, currentFrame);
}

void Stage3TransitionState::onEnter(BaseEnemy &enemy)
{
	auto &boss = static_cast<SegfaultBoss &>(enemy);
	boss.setVelocityX(0.f);
	boss.setInvincible(true);
	boss.requestBluescreen();
	currentFrame = 0;
	frameTimer = 0.f;
}

void Stage3TransitionState::onExit(BaseEnemy &enemy)
{
	auto &boss = static_cast<SegfaultBoss &>(enemy);
	boss.setInvincible(false);
	timer = 0.f;
}

json Stage3TransitionState::serialize() const
{
	json j = EnemyState::serialize();
	j["type"] = "SegfaultStage3TransitionState";
	return j;
}

} // namespace segfault_boss
