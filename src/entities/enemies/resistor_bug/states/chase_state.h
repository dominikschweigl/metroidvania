#pragma once
#include "../../enemy_state.h"

namespace resistor_bug {

// Bug walks toward the player along the ground. Transitions to JumpAttack
// (if in attack range and off cooldown) or back to Idle (if the player escapes).
class ChaseState : public EnemyState {
  public:
	[[nodiscard]] EnemyState *update(float deltaTime, BaseEnemy &enemy, const World &world,
	                                 sf::Vector2f playerPos) override;
	void updateAnimation(float deltaTime, BaseEnemy &enemy) override;
	void onEnter(BaseEnemy &enemy) override;
	json serialize() const override;
};

} // namespace resistor_bug
