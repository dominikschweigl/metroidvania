#include "attack_state.h"
#include "../../../../core/audio_manager.h"
#include "../recursion_golem.h"
#include <cmath>

namespace recursion_golem {

EnemyState *AttackState::update(float deltaTime, BaseEnemy &enemy, const World & /*world*/, sf::Vector2f playerPos)
{
	auto &golem = static_cast<RecursionGolem &>(enemy);

	if (stateTimer < RecursionGolem::ATTACK_DUR * 0.4f) {
		const float sign = (golem.getDirection() == Direction::Right) ? 1.f : -1.f;
		golem.setVelocityX(sign * RecursionGolem::ATTACK_LUNGE_SPEED);
	} else {
		golem.setVelocityX(0.f);
	}

	stateTimer += deltaTime;
	if (stateTimer >= RecursionGolem::ATTACK_DUR) {
		const float dist = std::abs(playerPos.x - golem.getPosition().x);
		if (dist < RecursionGolem::DETECT_RANGE)
			return &golem.states.chase;
		return &golem.states.idle;
	}

	return this;
}

void AttackState::updateAnimation(float deltaTime, BaseEnemy &enemy)
{
	auto &golem = static_cast<RecursionGolem &>(enemy);

	constexpr int FRAME_COUNT = 12;
	constexpr float FRAME_DURATION = RecursionGolem::ATTACK_DUR / FRAME_COUNT;

	frameTimer += deltaTime;
	if (frameTimer >= FRAME_DURATION) {
		frameTimer -= FRAME_DURATION;
		currentFrame = (currentFrame + 1) % FRAME_COUNT;
	}

	golem.setAnimation(RecursionGolem::GolemAnimation::Attack, currentFrame);
}

void AttackState::onEnter(BaseEnemy &enemy)
{
	auto &golem = static_cast<RecursionGolem &>(enemy);
	AudioManager::getInstance().playSound(SoundEffect::SLIME_ATTACK);
	stateTimer = 0.f;
	currentFrame = 0;
	frameTimer = 0.f;
	// The cooldown is armed by WindUpState; here we just open the damage window.
	golem.beginAttackSource();
}

void AttackState::onExit(BaseEnemy &enemy)
{
	static_cast<RecursionGolem &>(enemy).markAttackSourceEnded();
}

json AttackState::serialize() const
{
	json j = EnemyState::serialize();
	j["type"] = "GolemAttackState";
	return j;
}

} // namespace recursion_golem
