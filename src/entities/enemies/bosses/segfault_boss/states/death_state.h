#pragma once

#include "../../../enemy_state.h"

namespace segfault_boss {

// Death sequence after the boss is defeated.
// Moves afterwards to victory screen
class DeathState : public EnemyState {
  public:
	[[nodiscard]] EnemyState *update(float deltaTime, BaseEnemy &enemy, const World &world,
	                                 sf::Vector2f playerPos) override;
	void updateAnimation(float deltaTime, BaseEnemy &enemy) override;
	void onEnter(BaseEnemy &enemy) override;
	json serialize() const override;

  private:
	bool victorySignaled = false;
};

} // namespace segfault_boss
