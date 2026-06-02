#include "roaming_state.h"
#include "../../../../../core/audio_manager.h"
#include "../../../../entity_physics.h"
#include "../transistor_boss.h"

namespace transistor_boss {

EnemyState *RoamingState::update(float deltaTime, BaseEnemy &enemy, const World &world, sf::Vector2f playerPos)
{
	auto &transistor_boss = static_cast<TransistorBoss &>(enemy);

	moveTimer += deltaTime;

	float deltaX = playerPos.x - transistor_boss.getPosition().x;
	float distX = std::abs(deltaX);

	float deltaY = playerPos.y - transistor_boss.getPosition().y;
	float distY = std::abs(deltaY);

	if (!transistor_boss.isChargeOnCooldown() && distX < TransistorBoss::CHARGE_RANGE
	    && distY < TransistorBoss::CHARGE_RANGE) {
		return &transistor_boss.states.chargeAttackWindup;
	}

	if (!transistor_boss.isShootOnCooldown() && distX < TransistorBoss::SHOOT_RANGE
	    && distY < TransistorBoss::SHOOT_RANGE) {
		return &transistor_boss.states.shootAttack;
	}

	if (moveTimer > TransistorBoss::MOVE_DIRECTION_DUR && rand() % 2 == 1) {
		moveSign = -moveSign;
		moveTimer = 0;
	}

	// Turn around if cliff is in front.
	const sf::Vector2f pos = transistor_boss.getPosition();
	constexpr float LOOK_AHEAD = 8.f;
	const float probeX = pos.x + moveSign * (TransistorBoss::ENTITY_WIDTH / 2.f + LOOK_AHEAD);
	const sf::FloatRect groundProbe({probeX, pos.y - 1.f}, {1.f, 1.f});
	const bool cliffAhead = transistor_boss.isOnGroundFlag() && !EntityPhysics::isGroundBelow(groundProbe, world);

	const bool wallAhead =
	    (moveSign > 0.f)
	        ? EntityPhysics::isWallOnRight(pos, TransistorBoss::ENTITY_WIDTH, TransistorBoss::ENTITY_HEIGHT, world)
	        : EntityPhysics::isWallOnLeft(pos, TransistorBoss::ENTITY_WIDTH, TransistorBoss::ENTITY_HEIGHT, world);

	if (cliffAhead || wallAhead) {
		moveSign = -moveSign;
		moveTimer = 0;
	}

	transistor_boss.setVelocityX(moveSign * TransistorBoss::MOVE_SPEED);

	return this;
}

void RoamingState::updateAnimation(float deltaTime, BaseEnemy &enemy)
{
	auto &transistor_boss = static_cast<TransistorBoss &>(enemy);

	constexpr int FRAME_COUNT = 16;
	constexpr float FRAME_DURATION = 0.1f;

	// Footfalls land on every leg movement of the walk cycle.
	constexpr int STEP_FRAME_INTERVAL = 3;

	frameTimer += deltaTime;
	if (frameTimer >= FRAME_DURATION) {
		frameTimer -= FRAME_DURATION;
		currentFrame = (currentFrame + 1) % FRAME_COUNT;

		if (currentFrame % STEP_FRAME_INTERVAL == 0) {
			AudioManager::getInstance().playSound(SoundEffect::TRANSISTOR_BOSS_STEP);
		}
	}

	transistor_boss.setAnimation(TransistorBoss::TransistorBossAnimation::Roaming, currentFrame);
}

void RoamingState::onEnter(BaseEnemy &enemy)
{
	auto &transistor_boss = static_cast<TransistorBoss &>(enemy);
	transistor_boss.setVelocityX(0.f);
	moveSign = (rand() % 2 == 0) ? 1.f : -1.f;
	currentFrame = 0;
	frameTimer = 0.f;
}

void RoamingState::onExit(BaseEnemy &enemy)
{
	moveTimer = 0;
}

} // namespace transistor_boss