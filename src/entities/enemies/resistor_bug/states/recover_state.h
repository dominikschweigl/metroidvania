#pragma once
#include "../../enemy_state.h"

namespace resistor_bug {

// Bug recovers after landing a hop. Stationary, transitions to Chase if the player
// is still in range, otherwise back to Idle.
class RecoverState : public EnemyState {
  public:
	[[nodiscard]] EnemyState *update(float deltaTime, BaseEnemy &enemy, const World &world,
	                                 sf::Vector2f playerPos) override;
	void updateAnimation(float deltaTime, BaseEnemy &enemy) override;
	void onEnter(BaseEnemy &enemy) override;
	json serialize() const override;

  private:
	float stateTimer = 0.f;
};

} // namespace resistor_bug
