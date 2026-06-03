#pragma once
#include "../../enemy_state.h"

namespace capacitor {

// After shooting, capacitor will swoop closer to player
// Allows for punishment window, where capacitor can be hit
class SwoopState : public EnemyState {
  public:
	[[nodiscard]] EnemyState *update(float deltaTime, BaseEnemy &enemy, const World &world,
	                                 sf::Vector2f playerPos) override;
	void updateAnimation(float deltaTime, BaseEnemy &enemy) override;
	void onEnter(BaseEnemy &enemy) override;

  private:
	float timer = 0.f;
	float lingerTimer = 0.f;
	bool arrived = false;
};

} // namespace capacitor
