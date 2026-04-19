#pragma once
#include "player_state.h"
#include <SFML/Graphics.hpp>

class Player;

class PreJumpState : public PlayerState {
  public:
	const sf::Texture jump_texture{"./assets/images/player/jump.png"};

	static constexpr float PREJUMP_FRAME_DURATION = 0.08f;

	PlayerState *update(float dt, Player &p) override;
	void applyAnimation(float dt, Player &p) override;
	void onEnter(Player &p) override;

  private:
	bool readyToAscend = false;
};
