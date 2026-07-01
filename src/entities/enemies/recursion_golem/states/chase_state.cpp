#include "chase_state.h"
#include "../recursion_golem.h"
#include <cmath>

namespace recursion_golem {

EnemyState *ChaseState::update(float /*deltaTime*/, BaseEnemy &enemy, const World & /*world*/, sf::Vector2f playerPos)
{
	auto &golem = static_cast<RecursionGolem &>(enemy);

	const float deltaX = playerPos.x - golem.getPosition().x;
	const float dist = std::abs(deltaX);
	const float heightDiff = golem.getPosition().y - playerPos.y; // positive when player is above

	// Melee range: attack if ready, otherwise wait out the cooldown in Idle.
	if (dist < RecursionGolem::ATTACK_RANGE && std::abs(heightDiff) < RecursionGolem::ATTACK_RANGE) {
		golem.setVelocityX(0.f);
		if (golem.getAttackCooldown() <= 0.f)
			return &golem.states.windup;
		return &golem.states.idle;
	}

	// Player escaped: give up.
	if (dist > RecursionGolem::LOSE_RANGE) {
		golem.setVelocityX(0.f);
		return &golem.states.idle;
	}

	const float sign = (golem.getDirection() == Direction::Right) ? 1.f : -1.f;
	golem.setVelocityX(sign * golem.moveSpeed());
	golem.tryJumpTowards(heightDiff);

	return this;
}

void ChaseState::updateAnimation(float deltaTime, BaseEnemy &enemy)
{
	auto &golem = static_cast<RecursionGolem &>(enemy);

	constexpr int FRAME_COUNT = 8;
	constexpr float FRAME_DURATION = 0.1f;

	frameTimer += deltaTime;
	if (frameTimer >= FRAME_DURATION) {
		frameTimer -= FRAME_DURATION;
		currentFrame = (currentFrame + 1) % FRAME_COUNT;
	}

	golem.setAnimation(RecursionGolem::GolemAnimation::Moving, currentFrame);
}

void ChaseState::onEnter(BaseEnemy & /*enemy*/)
{
	currentFrame = 0;
	frameTimer = 0.f;
}

json ChaseState::serialize() const
{
	json j = EnemyState::serialize();
	j["type"] = "GolemChaseState";
	return j;
}

} // namespace recursion_golem
