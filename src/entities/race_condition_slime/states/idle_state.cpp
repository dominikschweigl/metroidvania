#include "../race_condition_slime.h"
#include "idle_state.h"
#include <cmath>

namespace rc_slime {

EnemyState *IdleState::update(float dt, BaseEnemy &enemy, const World &world, sf::Vector2f playerPos)
{
	auto &slime = static_cast<RaceConditionSlime &>(enemy);

	slime.setVelocityX(0.f);

	float dist = std::abs(playerPos.x - slime.getPosition().x);

	// Stay idle while finishing the post-attack cooldown and still in the players reach
	bool waitingForCooldown = slime.getAttackCooldown() > 0.f && dist < RaceConditionSlime::ATTACK_RANGE;

	if (dist < RaceConditionSlime::DETECT_RANGE && !waitingForCooldown) {
		return &slime.states.chase;
	}

	return this;
}

void IdleState::updateAnimation(float deltaTime, BaseEnemy &enemy)
{
	auto &slime = static_cast<RaceConditionSlime &>(enemy);

	constexpr float FRAME_DURATION = 0.1f;
	constexpr int FRAME_COUNT = 8;

	frameTimer += deltaTime;
	if (frameTimer >= FRAME_DURATION) {
		frameTimer -= FRAME_DURATION;
		currentFrame = (currentFrame + 1) % FRAME_COUNT;
	}

	slime.sprite.setTexture(slime.idleTexture);
	slime.sprite.setTextureRect(sf::IntRect({currentFrame * RaceConditionSlime::FRAME_SIZE, 0},
	                                        {RaceConditionSlime::FRAME_SIZE, RaceConditionSlime::FRAME_SIZE}));
}

void IdleState::onEnter(BaseEnemy &enemy)
{
	auto &slime = static_cast<RaceConditionSlime &>(enemy);
	slime.setVelocityX(0.f);
	currentFrame = 0;
	frameTimer = 0.f;
}

} // namespace rc_slime
