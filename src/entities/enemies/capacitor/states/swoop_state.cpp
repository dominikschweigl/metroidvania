#include "swoop_state.h"
#include "../capacitor.h"
#include <cmath>

namespace capacitor {

EnemyState *SwoopState::update(float deltaTime, BaseEnemy &enemy, const World &world, sf::Vector2f playerPos)
{
	auto &capacitor = static_cast<Capacitor &>(enemy);
	timer += deltaTime;

	const sf::Vector2f target = capacitor.swoopTarget(playerPos);
	sf::Vector2f toTarget = target - capacitor.getPosition();
	const float dist = std::hypot(toTarget.x, toTarget.y);

	if (!arrived && dist <= Capacitor::SWOOP_ARRIVE)
		arrived = true;

	if (arrived) {
		capacitor.setVelocity(capacitor.getVelocity() * 0.55f);
		lingerTimer += deltaTime;
		if (lingerTimer >= Capacitor::SWOOP_LINGER)
			return &capacitor.states.flee;
	} else if (dist > 0.0001f) {
		toTarget /= dist;
		capacitor.setVelocity(toTarget * Capacitor::SWOOP_SPEED);
	}

	if (timer >= Capacitor::SWOOP_MAX_DUR)
		return &capacitor.states.flee;

	return this;
}

void SwoopState::updateAnimation(float /*deltaTime*/, BaseEnemy & /*enemy*/) {}

void SwoopState::onEnter(BaseEnemy &enemy)
{
	timer = 0.f;
	static_cast<Capacitor &>(enemy).resetSwoopCounter();
}

json SwoopState::serialize() const
{
	json j = EnemyState::serialize();

	j["type"] = "SwoopState";

	return j;
}

} // namespace capacitor
