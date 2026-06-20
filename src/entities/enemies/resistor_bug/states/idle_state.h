#pragma once
#include "../../enemy_state.h"

namespace resistor_bug {

// Bug waits. Transitions to Chase when the player enters detection range.
class IdleState : public EnemyState {
  public:
	[[nodiscard]] EnemyState *update(float deltaTime, BaseEnemy &enemy, const World &world,
	                                 sf::Vector2f playerPos) override;
	void updateAnimation(float deltaTime, BaseEnemy &enemy) override;
	void onEnter(BaseEnemy &enemy) override;
	json serialize() const override;
};

} // namespace resistor_bug
