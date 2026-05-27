#include "recover_state.h"
#include "../race_condition_slime.h"
#include <cmath>

namespace rc_slime {

EnemyState *RecoverState::update(float deltaTime, BaseEnemy &enemy, const World &world, sf::Vector2f playerPos)
{
	auto &slime = static_cast<RaceConditionSlime &>(enemy);
	slime.setVelocityX(0.f);

	stateTimer += deltaTime;
	if (stateTimer >= RaceConditionSlime::RECOVER_DUR) {
		float dist = std::abs(playerPos.x - slime.getPosition().x);
		return (dist < RaceConditionSlime::DETECT_RANGE) ? static_cast<EnemyState *>(&slime.states.chase)
		                                                 : static_cast<EnemyState *>(&slime.states.idle);
	}
	return this;
}

void RecoverState::updateAnimation(float deltaTime, BaseEnemy &enemy)
{
	auto &slime = static_cast<RaceConditionSlime &>(enemy);

	constexpr int FRAME_COUNT = 8;
	constexpr float FRAME_DURATION = RaceConditionSlime::RECOVER_DUR / FRAME_COUNT;

	frameTimer += deltaTime;
	if (frameTimer >= FRAME_DURATION) {
		frameTimer -= FRAME_DURATION;
		currentFrame = (currentFrame + 1) % FRAME_COUNT;
	}

	slime.setAnimation(RaceConditionSlime::SlimeAnimation::Recover, currentFrame);
}

void RecoverState::onEnter(BaseEnemy & /*enemy*/)
{
	stateTimer = 0.f;
	currentFrame = 0;
	frameTimer = 0.f;
}

} // namespace rc_slime
