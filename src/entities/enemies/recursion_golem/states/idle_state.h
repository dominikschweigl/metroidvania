#pragma once
#include "../../enemy_state.h"

namespace recursion_golem {

// Golem waits. Transitions to Chase when the player enters detect range.
class IdleState : public EnemyState {
  public:
	[[nodiscard]] EnemyState *update(float deltaTime, BaseEnemy &enemy, const World &world,
	                                 sf::Vector2f playerPos) override;
	void updateAnimation(float deltaTime, BaseEnemy &enemy) override;
	void onEnter(BaseEnemy &enemy) override;
	json serialize() const override;
};

} // namespace recursion_golem
