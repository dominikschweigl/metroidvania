#pragma once

#include "../../../enemy_state.h"

namespace transistor_boss {

class ShootAttackState : public EnemyState {
  public:
	[[nodiscard]] EnemyState *update(float deltaTime, BaseEnemy &enemy, const World &world,
	                                 sf::Vector2f playerPos) override;
	void updateAnimation(float deltaTime, BaseEnemy &enemy) override;
	void onEnter(BaseEnemy &enemy) override;
	void onExit(BaseEnemy &enemy) override;
	json serialize() const override;

  private:
	float shotTimer = 0.f;
	int shotsFired = 0;
	float frameTimer = 0.f;
	int currentFrame = 0;
};

} // namespace transistor_boss
