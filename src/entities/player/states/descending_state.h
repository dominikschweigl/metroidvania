#pragma once
#include "../../../core/asset_manager.h"
#include "player_state.h"

class Player;

class DescendingState : public PlayerState {
  public:
	const sf::Texture &jump_lower_texture;
	const sf::Texture &jump_upper_texture;

	DescendingState();

	PlayerState *update(float dt, Player &p) override;
	void applyAnimation(float dt, Player &p) override;
	void onEnter(Player &p) override;
};
