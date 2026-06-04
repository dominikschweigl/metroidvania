#pragma once

#include "../../../enemy_state.h"

namespace transistor_boss {

class RoamingState : public EnemyState {

  public:
	[[nodiscard]] EnemyState *update(float deltaTime, BaseEnemy &enemy, const World &world,
	                                 sf::Vector2f playerPos) override;
	void updateAnimation(float deltaTime, BaseEnemy &enemy) override;
	void onEnter(BaseEnemy &enemy) override;
	void onExit(BaseEnemy &enemy) override;

  private:
	float moveTimer = 0;
	float moveSign = 1.f;
};

} // namespace transistor_boss