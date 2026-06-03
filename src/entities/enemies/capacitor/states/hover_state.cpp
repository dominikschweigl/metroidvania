#include "hover_state.h"
#include "../capacitor.h"
#include <cmath>

namespace capacitor {

EnemyState *HoverState::update(float deltaTime, BaseEnemy &enemy, const World &world, sf::Vector2f playerPos)
{
	auto &capacitor = static_cast<Capacitor &>(enemy);

	const sf::Vector2f pos = capacitor.getPosition();
	const float dist = std::hypot(playerPos.x - pos.x, playerPos.y - pos.y);

	if (dist > Capacitor::DETECT_RANGE) {
		capacitor.setVelocity(capacitor.getVelocity() * 0.9f);
		return this;
	}

	const sf::Vector2f target = capacitor.hoverTarget(playerPos);
	sf::Vector2f toTarget = target - pos;
	const float targetDist = std::hypot(toTarget.x, toTarget.y);
	if (targetDist > Capacitor::ARRIVE_DEADZONE) {
		toTarget /= targetDist;
		capacitor.setVelocity(toTarget * Capacitor::MOVE_SPEED);
	} else {
		capacitor.setVelocity(capacitor.getVelocity() * 0.8f);
	}

	if (!capacitor.isShootOnCooldown())
		return &capacitor.states.shoot;

	return this;
}

void HoverState::updateAnimation(float /*deltaTime*/, BaseEnemy & /*enemy*/) {}

} // namespace capacitor
