#pragma once
#include "../../enemy_state.h"

namespace recursion_golem {

// Stack Overflow: a defeated base-case golem stands and flashes through a short
// countdown while RecursionGolem::onPreUpdate drives the blast and its removal.
class ExplodeState : public EnemyState {
  public:
	[[nodiscard]] EnemyState *update(float deltaTime, BaseEnemy &enemy, const World &world,
	                                 sf::Vector2f playerPos) override;
	void updateAnimation(float deltaTime, BaseEnemy &enemy) override;
	void onEnter(BaseEnemy &enemy) override;
	json serialize() const override;
};

} // namespace recursion_golem
