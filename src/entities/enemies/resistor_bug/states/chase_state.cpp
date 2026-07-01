#include "chase_state.h"
#include "../resistor_bug.h"
#include <cmath>

namespace resistor_bug {

EnemyState *ChaseState::update(float /*deltaTime*/, BaseEnemy &enemy, const World & /*world*/, sf::Vector2f playerPos)
{
	auto &bug = static_cast<ResistorBug &>(enemy);

	const float dist = std::abs(playerPos.x - bug.getPosition().x);

	if (dist < ResistorBug::ATTACK_RANGE) {
		bug.setVelocityX(0.f);
		if (bug.getAttackCooldown() <= 0.f && bug.isOnGroundFlag()) {
			return &bug.states.jumpAttack;
		}
		return this;
	}

	if (dist > ResistorBug::LOSE_RANGE) {
		bug.setVelocityX(0.f);
		return &bug.states.idle;
	}

	const float sign = (bug.getDirection() == Direction::Right) ? 1.f : -1.f;
	bug.setVelocityX(sign * ResistorBug::MOVE_SPEED);

	return this;
}

void ChaseState::updateAnimation(float deltaTime, BaseEnemy &enemy)
{
	auto &bug = static_cast<ResistorBug &>(enemy);

	constexpr int FRAME_COUNT = 8;
	constexpr float FRAME_DURATION = 0.08f;

	frameTimer += deltaTime;
	if (frameTimer >= FRAME_DURATION) {
		frameTimer -= FRAME_DURATION;
		currentFrame = (currentFrame + 1) % FRAME_COUNT;
	}

	bug.setAnimation(ResistorBug::BugAnimation::Moving, currentFrame);
}

void ChaseState::onEnter(BaseEnemy & /*enemy*/)
{
	currentFrame = 0;
	frameTimer = 0.f;
}

json ChaseState::serialize() const
{
	json j = EnemyState::serialize();

	j["type"] = "ResistorChaseState";

	return j;
}

} // namespace resistor_bug
