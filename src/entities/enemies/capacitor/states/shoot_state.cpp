#include "shoot_state.h"
#include "../capacitor.h"

namespace capacitor {

EnemyState *ShootState::update(float deltaTime, BaseEnemy &enemy, const World &world, sf::Vector2f playerPos)
{
	auto &capacitor = static_cast<Capacitor &>(enemy);

	capacitor.setVelocity(capacitor.getVelocity() * 0.8f);

	if (!fired) {
		capacitor.spawnShot(playerPos);
		fired = true;
	}

	timer += deltaTime;
	if (timer >= Capacitor::SHOOT_DUR)
		return capacitor.shouldSwoop() ? static_cast<EnemyState *>(&capacitor.states.swoop)
		                               : static_cast<EnemyState *>(&capacitor.states.flee);

	return this;
}

void ShootState::updateAnimation(float /*deltaTime*/, BaseEnemy & /*enemy*/) {}

void ShootState::onEnter(BaseEnemy & /*enemy*/)
{
	timer = 0.f;
	fired = false;
}

void ShootState::onExit(BaseEnemy &enemy)
{
	static_cast<Capacitor &>(enemy).startShootCooldown();
}

} // namespace capacitor
