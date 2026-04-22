#pragma once
#include "../../base/enemy_state.h"

namespace rc_slime {

// Slime is anticipating an attack: stationary, brief wind-up.
class WindUpState : public EnemyState {
  public:
	[[nodiscard]] EnemyState *update(float deltaTime, BaseEnemy &enemy, const World &world,
	                                 sf::Vector2f playerPos) override;
	void updateAnimation(float deltaTime, BaseEnemy &enemy) override;
	void onEnter(BaseEnemy &enemy) override;

  private:
	float stateTimer = 0.f;
};

} // namespace rc_slime
