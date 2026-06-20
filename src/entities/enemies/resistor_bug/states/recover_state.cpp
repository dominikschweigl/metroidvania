#include "recover_state.h"
#include "../resistor_bug.h"
#include <cmath>

namespace resistor_bug {

EnemyState *RecoverState::update(float deltaTime, BaseEnemy &enemy, const World &world, sf::Vector2f playerPos)
{
	auto &bug = static_cast<ResistorBug &>(enemy);
	bug.setVelocityX(0.f);

	stateTimer += deltaTime;
	if (stateTimer >= ResistorBug::RECOVER_DUR) {
		const float dist = std::abs(playerPos.x - bug.getPosition().x);
		return (dist < ResistorBug::DETECT_RANGE) ? static_cast<EnemyState *>(&bug.states.chase)
		                                          : static_cast<EnemyState *>(&bug.states.idle);
	}
	return this;
}

void RecoverState::updateAnimation(float deltaTime, BaseEnemy &enemy)
{
	auto &bug = static_cast<ResistorBug &>(enemy);

	constexpr int FRAME_COUNT = 8;
	constexpr float FRAME_DURATION = ResistorBug::RECOVER_DUR / FRAME_COUNT;

	frameTimer += deltaTime;
	if (frameTimer >= FRAME_DURATION) {
		frameTimer -= FRAME_DURATION;
		currentFrame = (currentFrame + 1) % FRAME_COUNT;
	}

	bug.setAnimation(ResistorBug::BugAnimation::Recover, currentFrame);
}

void RecoverState::onEnter(BaseEnemy & /*enemy*/)
{
	stateTimer = 0.f;
	currentFrame = 0;
	frameTimer = 0.f;
}

json RecoverState::serialize() const
{
	json j = EnemyState::serialize();

	j["type"] = "ResistorRecoverState";

	return j;
}

} // namespace resistor_bug
