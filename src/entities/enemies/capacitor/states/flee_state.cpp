#include "flee_state.h"
#include "../capacitor.h"
#include <cmath>

namespace capacitor {

EnemyState *FleeState::update(float deltaTime, BaseEnemy &enemy, const World & /*world*/, sf::Vector2f playerPos)
{
	auto &capacitor = static_cast<Capacitor &>(enemy);
	timer += deltaTime;

	const sf::Vector2f pos = capacitor.getPosition();
	const float distToPlayer = std::hypot(pos.x - playerPos.x, pos.y - playerPos.y);
	if (distToPlayer >= Capacitor::FLEE_DISTANCE || timer >= Capacitor::FLEE_DUR)
		return &capacitor.states.hover;

	const float awayX = (pos.x >= playerPos.x) ? 1.f : -1.f;
	sf::Vector2f flee{awayX, -Capacitor::FLEE_RISE};
	const float length = std::hypot(flee.x, flee.y);
	flee /= length;
	capacitor.setVelocity(flee * Capacitor::FLEE_SPEED);

	return this;
}

void FleeState::updateAnimation(float /*deltaTime*/, BaseEnemy & /*enemy*/) {}

void FleeState::onEnter(BaseEnemy & /*enemy*/)
{
	timer = 0.f;
}

json FleeState::serialize() const
{
	json j = EnemyState::serialize();

	j["type"] = "FleeState";

	return j;
}

} // namespace capacitor
