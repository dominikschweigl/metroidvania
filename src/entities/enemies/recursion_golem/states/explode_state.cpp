#include "explode_state.h"
#include "../recursion_golem.h"

namespace recursion_golem {

EnemyState *ExplodeState::update(float /*deltaTime*/, BaseEnemy &enemy, const World & /*world*/,
                                 sf::Vector2f /*playerPos*/)
{
	// The golem braces in place; the countdown and blast live on the golem itself.
	static_cast<RecursionGolem &>(enemy).setVelocityX(0.f);
	return this;
}

void ExplodeState::updateAnimation(float deltaTime, BaseEnemy &enemy)
{
	auto &golem = static_cast<RecursionGolem &>(enemy);

	if (!golem.isExplosionFired()) {
		constexpr int FRAME_COUNT = 8;
		constexpr float FRAME_DURATION = RecursionGolem::EXPLODE_COUNTDOWN / FRAME_COUNT;

		frameTimer += deltaTime;
		if (frameTimer >= FRAME_DURATION) {
			frameTimer -= FRAME_DURATION;
			currentFrame = (currentFrame + 1) % FRAME_COUNT;
		}

		golem.setAnimation(RecursionGolem::GolemAnimation::Explode, currentFrame);
	} else {
		golem.setAnimation(RecursionGolem::GolemAnimation::Explosion, golem.explosionAnimFrame());
	}
}

void ExplodeState::onEnter(BaseEnemy &enemy)
{
	static_cast<RecursionGolem &>(enemy).setVelocityX(0.f);
	currentFrame = 0;
	frameTimer = 0.f;
}

json ExplodeState::serialize() const
{
	json j = EnemyState::serialize();
	j["type"] = "GolemExplodeState";
	return j;
}

} // namespace recursion_golem
