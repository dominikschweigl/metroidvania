#include "roaming_state.h"
#include "../../../../entity_physics.h"
#include "../segfault_boss.h"

#include <cmath>
#include <cstdlib>

namespace segfault_boss {

EnemyState *RoamingState::update(float deltaTime, BaseEnemy &enemy, const World &world, sf::Vector2f playerPos)
{
	auto &boss = static_cast<SegfaultBoss &>(enemy);

	// Stage transitions only apply to the original process, never to a clone.
	if (!boss.isClone()) {
		// Crossing into stage two: pause, then summon a wave of processes (once).
		if (!boss.isStage2Triggered() && boss.health.current <= SegfaultBoss::STAGE2_HP)
			return &boss.states.stage2Transition;

		// Crossing into stage three: bluescreen interrupt, then fork a clone (once).
		if (boss.isStage2Triggered() && !boss.isStage3Triggered() &&
		    boss.health.current <= SegfaultBoss::STAGE3_HP)
			return &boss.states.stage3Transition;
	}

	if (!boss.isSpearOnCooldown() && std::abs(playerPos.x - boss.getPosition().x) < SegfaultBoss::SPEAR_RANGE)
		return &boss.states.nullSpearAttack;

	moveTimer += deltaTime;

	// Drift toward the player, flipping to a random patrol direction regularly
	if (moveTimer > SegfaultBoss::MOVE_DIRECTION_DUR && rand() % 2 == 1) {
		moveSign = -moveSign;
		moveTimer = 0.f;
	}

	const sf::Vector2f pos = boss.getPosition();
	constexpr float LOOK_AHEAD = 8.f;
	const float probeX = pos.x + moveSign * (SegfaultBoss::ENTITY_WIDTH / 2.f + LOOK_AHEAD);
	const sf::FloatRect groundProbe({probeX, pos.y - 1.f}, {1.f, 1.f});
	const bool cliffAhead = boss.isOnGroundFlag() && !EntityPhysics::isGroundBelow(groundProbe, world);

	const bool wallAhead =
	    (moveSign > 0.f)
	        ? EntityPhysics::isWallOnRight(pos, SegfaultBoss::ENTITY_WIDTH, SegfaultBoss::ENTITY_HEIGHT, world)
	        : EntityPhysics::isWallOnLeft(pos, SegfaultBoss::ENTITY_WIDTH, SegfaultBoss::ENTITY_HEIGHT, world);

	if (cliffAhead || wallAhead) {
		moveSign = -moveSign;
		moveTimer = 0.f;
	}

	boss.setVelocityX(moveSign * SegfaultBoss::MOVE_SPEED);

	return this;
}

void RoamingState::updateAnimation(float deltaTime, BaseEnemy &enemy)
{
	auto &boss = static_cast<SegfaultBoss &>(enemy);

	constexpr int FRAME_COUNT = 16;
	constexpr float FRAME_DURATION = 0.1f;

	frameTimer += deltaTime;
	if (frameTimer >= FRAME_DURATION) {
		frameTimer -= FRAME_DURATION;
		currentFrame = (currentFrame + 1) % FRAME_COUNT;
	}

	boss.setAnimation(SegfaultBoss::SegfaultBossAnimation::Roaming, currentFrame);
}

void RoamingState::onEnter(BaseEnemy &enemy)
{
	auto &boss = static_cast<SegfaultBoss &>(enemy);
	boss.setVelocityX(0.f);
	moveSign = (rand() % 2 == 0) ? 1.f : -1.f;
	currentFrame = 0;
	frameTimer = 0.f;
}

void RoamingState::onExit(BaseEnemy & /*enemy*/)
{
	moveTimer = 0.f;
}

json RoamingState::serialize() const
{
	json j = EnemyState::serialize();
	j["type"] = "SegfaultRoamingState";
	return j;
}

} // namespace segfault_boss
