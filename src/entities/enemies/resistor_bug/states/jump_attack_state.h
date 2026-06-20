#pragma once
#include "../../enemy_state.h"

namespace resistor_bug {

// Bug telegraphs the strike while stationary, then hops toward the player. Its body is the
// active damage hitbox while airborne. Transitions to Recover once it lands.
class JumpAttackState : public EnemyState {
  public:
	[[nodiscard]] EnemyState *update(float deltaTime, BaseEnemy &enemy, const World &world,
	                                 sf::Vector2f playerPos) override;
	void updateAnimation(float deltaTime, BaseEnemy &enemy) override;
	void onEnter(BaseEnemy &enemy) override;
	void onExit(BaseEnemy &enemy) override;
	json serialize() const override;

  private:
	float stateTimer = 0.f;
	bool hopLaunched = false;
};

} // namespace resistor_bug
