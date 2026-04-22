#include "attack_state.h"
#include "../race_condition_slime.h"

namespace rc_slime {

EnemyState *AttackState::update(float deltaTime, BaseEnemy &enemy, const World &world, sf::Vector2f playerPos)
{
	auto &slime = static_cast<RaceConditionSlime &>(enemy);
	slime.setVelocityX(0.f);

	stateTimer += deltaTime;
	if (stateTimer >= RaceConditionSlime::ATTACK_DUR) {
		return &slime.states.recover;
	}
	return this;
}

void AttackState::updateAnimation(float deltaTime, BaseEnemy &enemy)
{
	auto &slime = static_cast<RaceConditionSlime &>(enemy);

	constexpr int FRAME_COUNT = 12;
	constexpr float FRAME_DURATION = RaceConditionSlime::ATTACK_DUR / FRAME_COUNT;

	frameTimer += deltaTime;
	if (frameTimer >= FRAME_DURATION) {
		frameTimer -= FRAME_DURATION;
		currentFrame = (currentFrame + 1) % FRAME_COUNT;
	}

	slime.setAnimation(RaceConditionSlime::SlimeAnimation::Attack, currentFrame);
}

void AttackState::onEnter(BaseEnemy & /*enemy*/)
{
	stateTimer = 0.f;
	currentFrame = 0;
	frameTimer = 0.f;
}

} // namespace rc_slime
