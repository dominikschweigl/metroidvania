#pragma once
#include "../../../core/asset_manager.h"
#include "player_state.h"

class Player;

class PreJumpState : public PlayerState {
  public:
	const sf::Texture &jump_texture;

	PreJumpState();

	static constexpr float PREJUMP_FRAME_DURATION = 0.08f;

	PlayerState *update(float dt, Player &p) override;
	void applyAnimation(float dt, Player &p) override;
	void onEnter(Player &p) override;

  private:
	bool readyToAscend = false;
};
