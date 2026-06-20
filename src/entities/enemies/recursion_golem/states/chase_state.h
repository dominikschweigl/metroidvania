#pragma once
#include "../../enemy_state.h"

namespace recursion_golem {

// Golem walks toward the player. Transitions to Attack (in range, off cooldown)
// or Idle (player escapes / still on cooldown in range).
class ChaseState : public EnemyState {
  public:
	[[nodiscard]] EnemyState *update(float deltaTime, BaseEnemy &enemy, const World &world,
	                                 sf::Vector2f playerPos) override;
	void updateAnimation(float deltaTime, BaseEnemy &enemy) override;
	void onEnter(BaseEnemy &enemy) override;
	json serialize() const override;
};

} // namespace recursion_golem
