#include "windup_state.h"
#include "../recursion_golem.h"

namespace recursion_golem {

EnemyState *WindUpState::update(float deltaTime, BaseEnemy &enemy, const World & /*world*/, sf::Vector2f /*playerPos*/)
{
	auto &golem = static_cast<RecursionGolem &>(enemy);
	golem.setVelocityX(0.f);

	stateTimer += deltaTime;
	if (stateTimer >= RecursionGolem::WINDUP_DUR)
		return &golem.states.attack;

	return this;
}

void WindUpState::updateAnimation(float deltaTime, BaseEnemy &enemy)
{
	auto &golem = static_cast<RecursionGolem &>(enemy);

	constexpr int FRAME_COUNT = 4;
	constexpr float FRAME_DURATION = RecursionGolem::WINDUP_DUR / FRAME_COUNT;

	frameTimer += deltaTime;
	if (frameTimer >= FRAME_DURATION) {
		frameTimer -= FRAME_DURATION;
		currentFrame = (currentFrame + 1) % FRAME_COUNT;
	}

	golem.setAnimation(RecursionGolem::GolemAnimation::WindUp, currentFrame);
}

void WindUpState::onEnter(BaseEnemy &enemy)
{
	auto &golem = static_cast<RecursionGolem &>(enemy);
	golem.setVelocityX(0.f);
	golem.setAttackCooldown(RecursionGolem::ATTACK_COOLDOWN);
	stateTimer = 0.f;
	currentFrame = 0;
	frameTimer = 0.f;
}

json WindUpState::serialize() const
{
	json j = EnemyState::serialize();
	j["type"] = "GolemWindUpState";
	return j;
}

} // namespace recursion_golem
