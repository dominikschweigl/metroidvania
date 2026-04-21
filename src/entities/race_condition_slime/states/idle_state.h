#pragma once
#include "../../base/enemy_state.h"

namespace rc_slime {

// Slime waits. Transitions to Chase when the player enters
class IdleState : public EnemyState {
  public:
	[[nodiscard]] EnemyState *update(float deltaTime, BaseEnemy &enemy, const World &world,
	                                 sf::Vector2f playerPos) override;
	void updateAnimation(float deltaTime, BaseEnemy &enemy) override;
	void onEnter(BaseEnemy &enemy) override;
};

} // namespace rc_slime
