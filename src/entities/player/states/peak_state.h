#pragma once
#include "../../../core/asset_manager.h"
#include "player_state.h"

class Player;

class PeakState : public PlayerState {
  public:
	const sf::Texture& jump_texture;

	PeakState();

	PlayerState *update(float dt, Player &p) override;
	void applyAnimation(float dt, Player &p) override;
	void onEnter(Player &p) override;
};
