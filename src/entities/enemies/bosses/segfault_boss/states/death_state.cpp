#include "death_state.h"
#include "../segfault_boss.h"

namespace {
constexpr int DEATH_FRAME_COUNT = 18;
constexpr float DEATH_FRAME_DURATION = 1.f / 6.f; // 6 fps
} // namespace

namespace segfault_boss {

EnemyState *DeathState::update(float /*deltaTime*/, BaseEnemy &enemy, const World & /*world*/,
                               sf::Vector2f /*playerPos*/)
{
	if (!victorySignaled && animationComplete) {
		auto &boss = static_cast<SegfaultBoss &>(enemy);
		boss.requestVictory();
		victorySignaled = true;
	}
	return this;
}

void DeathState::updateAnimation(float deltaTime, BaseEnemy &enemy)
{
	auto &boss = static_cast<SegfaultBoss &>(enemy);

	if (!animationComplete) {
		frameTimer += deltaTime;
		if (frameTimer >= DEATH_FRAME_DURATION) {
			frameTimer -= DEATH_FRAME_DURATION;
			if (currentFrame < DEATH_FRAME_COUNT - 1)
				++currentFrame;
			else
				animationComplete = true;
		}
	}

	boss.setAnimation(SegfaultBoss::SegfaultBossAnimation::Death, currentFrame);
}

void DeathState::onEnter(BaseEnemy &enemy)
{
	auto &boss = static_cast<SegfaultBoss &>(enemy);
	boss.setVelocityX(0.f);
	boss.setInvincible(true);
	currentFrame = 0;
	frameTimer = 0.f;
	victorySignaled = false;
	animationComplete = false;
}

json DeathState::serialize() const
{
	json j = EnemyState::serialize();
	j["type"] = "SegfaultDeathState";
	return j;
}

} // namespace segfault_boss
