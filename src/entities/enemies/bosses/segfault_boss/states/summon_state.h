#pragma once

#include "../../../enemy_state.h"

namespace segfault_boss {

// Stage-two opener: spawns a handful of random processes (slimes / capacitors)
// as boss-owned sub-entities, then returns to roaming.
class SummonState : public EnemyState {
  public:
	[[nodiscard]] EnemyState *update(float deltaTime, BaseEnemy &enemy, const World &world,
	                                 sf::Vector2f playerPos) override;
	void updateAnimation(float deltaTime, BaseEnemy &enemy) override;
	void onEnter(BaseEnemy &enemy) override;
	void onExit(BaseEnemy &enemy) override;
	json serialize() const override;

  private:
	float timer = 0.f;
};

} // namespace segfault_boss
