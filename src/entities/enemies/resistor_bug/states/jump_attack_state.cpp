#include "jump_attack_state.h"
#include "../resistor_bug.h"

namespace resistor_bug {

EnemyState *JumpAttackState::update(float deltaTime, BaseEnemy &enemy, const World &world, sf::Vector2f playerPos)
{
	auto &bug = static_cast<ResistorBug &>(enemy);

	stateTimer += deltaTime;

	if (!hopLaunched) {
		bug.setVelocityX(0.f);
		if (stateTimer >= ResistorBug::TELEGRAPH_DUR) {
			bug.launchHop(playerPos);
			hopLaunched = true;
		}
		return this;
	}

	if (bug.isOnGroundFlag() || stateTimer >= ResistorBug::JUMPATTACK_MAX_DUR) {
		return &bug.states.recover;
	}

	return this;
}

void JumpAttackState::updateAnimation(float deltaTime, BaseEnemy &enemy)
{
	auto &bug = static_cast<ResistorBug &>(enemy);

	const ResistorBug::BugAnimation anim =
	    hopLaunched ? ResistorBug::BugAnimation::Attack : ResistorBug::BugAnimation::Telegraph;

	constexpr int FRAME_COUNT = 8;
	constexpr float FRAME_DURATION = 0.06f;

	frameTimer += deltaTime;
	if (frameTimer >= FRAME_DURATION) {
		frameTimer -= FRAME_DURATION;
		currentFrame = (currentFrame + 1) % FRAME_COUNT;
	}

	bug.setAnimation(anim, currentFrame);
}

void JumpAttackState::onEnter(BaseEnemy &enemy)
{
	auto &bug = static_cast<ResistorBug &>(enemy);
	stateTimer = 0.f;
	currentFrame = 0;
	frameTimer = 0.f;
	hopLaunched = false;
	bug.setVelocityX(0.f);
	bug.setAttackCooldown(ResistorBug::ATTACK_COOLDOWN);
}

void JumpAttackState::onExit(BaseEnemy &enemy)
{
	static_cast<ResistorBug &>(enemy).endAttack();
}

json JumpAttackState::serialize() const
{
	json j = EnemyState::serialize();

	j["type"] = "ResistorJumpAttackState";

	return j;
}

} // namespace resistor_bug
