#pragma once
#include "../../base/enemy_state.h"

namespace rc_slime {

// Slime is striking. Stationary, brief, then transitions to Recover.
class AttackState : public EnemyState {
  public:
	[[nodiscard]] EnemyState *update(float deltaTime, BaseEnemy &enemy, const World &world,
	                                 sf::Vector2f playerPos) override;
	void updateAnimation(float deltaTime, BaseEnemy &enemy) override;
	void onEnter(BaseEnemy &enemy) override;

  private:
	float stateTimer = 0.f;
};

} // namespace rc_slime
