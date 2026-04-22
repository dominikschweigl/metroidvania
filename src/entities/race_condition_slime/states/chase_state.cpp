#include "chase_state.h"
#include "../race_condition_slime.h"
#include <cmath>

namespace rc_slime {

EnemyState *ChaseState::update(float deltaTime, BaseEnemy &enemy, const World &world, sf::Vector2f playerPos)
{
	auto &slime = static_cast<RaceConditionSlime &>(enemy);

	float deltaX = playerPos.x - slime.getPosition().x;
	float heightDiff = slime.getPosition().y - playerPos.y; // positive when player is higher
	float dist = std::abs(deltaX);

	// Melee range: attack (or stall if still on cooldown).
	if (dist < RaceConditionSlime::ATTACK_RANGE && std::abs(heightDiff) < RaceConditionSlime::ATTACK_RANGE) {
		slime.setVelocityX(0.f);
		if (slime.getAttackCooldown() <= 0.f) {
			return &slime.states.windup;
		}
		return &slime.states.idle;
	}

	// Lost line-of-sight: give up and idle.
	if (dist > RaceConditionSlime::LOSE_RANGE) {
		slime.setVelocityX(0.f);
		return &slime.states.idle;
	}

	// Move toward player.
	float direction = (slime.getFacing() == BaseEnemy::Direction::Right) ? 1.f : -1.f;
	slime.setVelocityX(direction * RaceConditionSlime::MOVE_SPEED);

	slime.tryJumpTowards(heightDiff);
	slime.maybeTeleport(world, playerPos);

	return this;
}

void ChaseState::updateAnimation(float deltaTime, BaseEnemy &enemy)
{
	auto &slime = static_cast<RaceConditionSlime &>(enemy);

	constexpr int FRAME_COUNT = 8;
	constexpr float FRAME_DURATION = 0.08f;

	frameTimer += deltaTime;
	if (frameTimer >= FRAME_DURATION) {
		frameTimer -= FRAME_DURATION;
		currentFrame = (currentFrame + 1) % FRAME_COUNT;
	}

	slime.setAnimation(RaceConditionSlime::SlimeAnimation::Moving, currentFrame);
}

void ChaseState::onEnter(BaseEnemy & /*enemy*/)
{
	currentFrame = 0;
	frameTimer = 0.f;
}

} // namespace rc_slime
