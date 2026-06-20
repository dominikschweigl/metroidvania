#pragma once

#include "../../../enemy_state.h"

namespace segfault_boss {

// NULL spear attack.
// 4-6 spears spawn temporarily around the player.
class NullSpearAttackState : public EnemyState {
  public:
	[[nodiscard]] EnemyState *update(float deltaTime, BaseEnemy &enemy, const World &world,
	                                 sf::Vector2f playerPos) override;
	void updateAnimation(float deltaTime, BaseEnemy &enemy) override;
	void onEnter(BaseEnemy &enemy) override;
	void onExit(BaseEnemy &enemy) override;
	json serialize() const override;

  private:
	int spawnTarget = 0;
	int spawned = 0;
	float spawnTimer = 0.f;
};

} // namespace segfault_boss
