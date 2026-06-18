#pragma once

#include "../../../enemy_state.h"

namespace segfault_boss {

// Invincible pause the moment the boss crosses into stage three. Raises the
// bluescreen request on entry so the scene can interrupt the fight.
class Stage3TransitionState : public EnemyState {
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
