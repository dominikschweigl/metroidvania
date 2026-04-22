#pragma once
#include "../../base/enemy_state.h"

namespace rc_slime {

// Slime is recovering after an attack. Stationary; transitions to Chase
// if player still in range or back to Idle.
class RecoverState : public EnemyState {
  public:
	[[nodiscard]] EnemyState *update(float deltaTime, BaseEnemy &enemy, const World &world,
	                                 sf::Vector2f playerPos) override;
	void updateAnimation(float deltaTime, BaseEnemy &enemy) override;
	void onEnter(BaseEnemy &enemy) override;

  private:
	float stateTimer = 0.f;
};

} // namespace rc_slime
