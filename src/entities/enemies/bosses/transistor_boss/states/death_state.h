#pragma once

#include "../../../enemy_state.h"

namespace transistor_boss {

// Death animation playing after boss is defeated.
// Plays animation sequence: windup -> recover -> windup -> explosion.
// Exploded body stays present in the room instead of being removed.
class DeathState : public EnemyState {
  public:
	[[nodiscard]] EnemyState *update(float deltaTime, BaseEnemy &enemy, const World &world,
	                                 sf::Vector2f playerPos) override;
	void updateAnimation(float deltaTime, BaseEnemy &enemy) override;
	void onEnter(BaseEnemy &enemy) override;

  private:
	enum class Phase { WindupFirst, Recover, WindupSecond, Explosion };

	Phase phase = Phase::WindupFirst;
	bool victoryMusicStarted = false;
};

} // namespace transistor_boss
