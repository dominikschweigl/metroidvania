#pragma once
#include "../../enemy_state.h"

namespace recursion_golem {

// Golem strikes. Stationary, brief. Publishes a contact hitbox, then returns to
// Chase (player still nearby) or Idle.
class AttackState : public EnemyState {
  public:
	[[nodiscard]] EnemyState *update(float deltaTime, BaseEnemy &enemy, const World &world,
	                                 sf::Vector2f playerPos) override;
	void updateAnimation(float deltaTime, BaseEnemy &enemy) override;
	void onEnter(BaseEnemy &enemy) override;
	void onExit(BaseEnemy &enemy) override;
	json serialize() const override;

  private:
	float stateTimer = 0.f;
};

} // namespace recursion_golem
