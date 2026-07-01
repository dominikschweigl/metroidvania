#include "idle_state.h"
#include "../recursion_golem.h"
#include <cmath>

namespace recursion_golem {

EnemyState *IdleState::update(float /*deltaTime*/, BaseEnemy &enemy, const World & /*world*/, sf::Vector2f playerPos)
{
	auto &golem = static_cast<RecursionGolem &>(enemy);
	golem.setVelocityX(0.f);

	const float dist = std::abs(playerPos.x - golem.getPosition().x);
	if (dist < RecursionGolem::DETECT_RANGE)
		return &golem.states.chase;

	return this;
}

void IdleState::updateAnimation(float deltaTime, BaseEnemy &enemy)
{
	auto &golem = static_cast<RecursionGolem &>(enemy);

	constexpr float FRAME_DURATION = 0.12f;
	constexpr int FRAME_COUNT = 8;

	frameTimer += deltaTime;
	if (frameTimer >= FRAME_DURATION) {
		frameTimer -= FRAME_DURATION;
		currentFrame = (currentFrame + 1) % FRAME_COUNT;
	}

	golem.setAnimation(RecursionGolem::GolemAnimation::Idle, currentFrame);
}

void IdleState::onEnter(BaseEnemy &enemy)
{
	static_cast<RecursionGolem &>(enemy).setVelocityX(0.f);
	currentFrame = 0;
	frameTimer = 0.f;
}

json IdleState::serialize() const
{
	json j = EnemyState::serialize();
	j["type"] = "GolemIdleState";
	return j;
}

} // namespace recursion_golem
