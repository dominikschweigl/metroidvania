#pragma once
#include "../../../core/asset_manager.h"
#include "player_state.h"

class Player;

class AscendingState : public PlayerState {
  public:
	const sf::Texture &jump_texture;

	AscendingState();

	PlayerState *update(float dt, Player &p) override;
	void applyAnimation(float dt, Player &p) override;
	void onEnter(Player &p) override;
};
