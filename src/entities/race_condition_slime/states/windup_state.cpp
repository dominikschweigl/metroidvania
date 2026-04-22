#include "windup_state.h"
#include "../race_condition_slime.h"

namespace rc_slime {

EnemyState *WindUpState::update(float deltaTime, BaseEnemy &enemy, const World &world, sf::Vector2f playerPos)
{
	auto &slime = static_cast<RaceConditionSlime &>(enemy);
	slime.setVelocityX(0.f);

	stateTimer += deltaTime;
	if (stateTimer >= RaceConditionSlime::WINDUP_DUR) {
		return &slime.states.attack;
	}
	return this;
}

void WindUpState::updateAnimation(float deltaTime, BaseEnemy &enemy)
{
	auto &slime = static_cast<RaceConditionSlime &>(enemy);

	constexpr int FRAME_COUNT = 8;
	constexpr float FRAME_DURATION = RaceConditionSlime::WINDUP_DUR / FRAME_COUNT;

	frameTimer += deltaTime;
	if (frameTimer >= FRAME_DURATION) {
		frameTimer -= FRAME_DURATION;
		currentFrame = (currentFrame + 1) % FRAME_COUNT;
	}

	slime.setAnimation(RaceConditionSlime::SlimeAnimation::WindUp, currentFrame);
}

void WindUpState::onEnter(BaseEnemy &enemy)
{
	auto &slime = static_cast<RaceConditionSlime &>(enemy);
	slime.setVelocityX(0.f);
	slime.setAttackCooldown(RaceConditionSlime::ATTACK_COOLDOWN);
	stateTimer = 0.f;
	currentFrame = 0;
	frameTimer = 0.f;
}

} // namespace rc_slime
