#pragma once
#include "../../base/enemy_state.h"

namespace rc_slime {

// Slime actively pursues the player. Handles horizontal movement and jumping
// Transitions to WindUp (if in attack range) or Idle (if player escapes).
class ChaseState : public EnemyState {
  public:
	[[nodiscard]] EnemyState *update(float deltaTime, BaseEnemy &enemy, const World &world,
	                                 sf::Vector2f playerPos) override;
	void updateAnimation(float deltaTime, BaseEnemy &enemy) override;
	void onEnter(BaseEnemy &enemy) override;
};

} // namespace rc_slime
