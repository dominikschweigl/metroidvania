#include "idle_state.h"
#include "../resistor_bug.h"
#include <cmath>

namespace resistor_bug {

EnemyState *IdleState::update(float deltaTime, BaseEnemy &enemy, const World &world, sf::Vector2f playerPos)
{
	auto &bug = static_cast<ResistorBug &>(enemy);

	bug.setVelocityX(0.f);

	const float dist = std::abs(playerPos.x - bug.getPosition().x);

	const bool waitingForCooldown = bug.getAttackCooldown() > 0.f && dist < ResistorBug::ATTACK_RANGE;

	if (dist < ResistorBug::DETECT_RANGE && !waitingForCooldown) {
		return &bug.states.chase;
	}

	return this;
}

void IdleState::updateAnimation(float deltaTime, BaseEnemy &enemy)
{
	auto &bug = static_cast<ResistorBug &>(enemy);

	constexpr float FRAME_DURATION = 0.1f;
	constexpr int FRAME_COUNT = 8;

	frameTimer += deltaTime;
	if (frameTimer >= FRAME_DURATION) {
		frameTimer -= FRAME_DURATION;
		currentFrame = (currentFrame + 1) % FRAME_COUNT;
	}

	bug.setAnimation(ResistorBug::BugAnimation::Idle, currentFrame);
}

void IdleState::onEnter(BaseEnemy &enemy)
{
	auto &bug = static_cast<ResistorBug &>(enemy);
	bug.setVelocityX(0.f);
	currentFrame = 0;
	frameTimer = 0.f;
}

json IdleState::serialize() const
{
	json j = EnemyState::serialize();

	j["type"] = "ResistorIdleState";

	return j;
}

} // namespace resistor_bug
