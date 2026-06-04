#pragma once

#include "../../../enemy_state.h"

namespace transistor_boss {

// Recover state entered the moment the boss crosses into stage two.
class Stage2RecoverState : public EnemyState {
  public:
	[[nodiscard]] EnemyState *update(float deltaTime, BaseEnemy &enemy, const World &world,
	                                 sf::Vector2f playerPos) override;
	void updateAnimation(float deltaTime, BaseEnemy &enemy) override;
	void onEnter(BaseEnemy &enemy) override;
	void onExit(BaseEnemy &enemy) override;

  private:
	float timer = 0.f;
};

} // namespace transistor_boss
