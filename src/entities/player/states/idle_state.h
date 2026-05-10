#pragma once
#include "../../../core/asset_manager.h"
#include "player_state.h"

class Player;

class IdleState : public PlayerState {
  public:
	const sf::Texture& idle_texture;
	const sf::Texture& idle_lower_texture;

	IdleState();
	bool canAttack() const noexcept override { return true; }
	PlayerState *update(float dt, Player &p) override;
	void applyAnimation(float dt, Player &p) override;
	void onEnter(Player &p) override;
};
