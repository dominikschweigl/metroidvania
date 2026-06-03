#pragma once
#include "../../enemy_state.h"

namespace capacitor {

class ShootState : public EnemyState {
  public:
	[[nodiscard]] EnemyState *update(float deltaTime, BaseEnemy &enemy, const World &world,
	                                 sf::Vector2f playerPos) override;
	void updateAnimation(float deltaTime, BaseEnemy &enemy) override;
	void onEnter(BaseEnemy &enemy) override;
	void onExit(BaseEnemy &enemy) override;

  private:
	float timer = 0.f;
	bool fired = false;
};

} // namespace capacitor
