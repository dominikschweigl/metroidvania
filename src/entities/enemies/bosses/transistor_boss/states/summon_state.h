#pragma once

#include "../../../enemy_state.h"

namespace transistor_boss {

// Boss opening state for stage 2. Spawns tethered Capacitors
// Returns to roaming and fighting normally afterwards.
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

} // namespace transistor_boss
