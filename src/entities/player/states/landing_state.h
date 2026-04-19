#pragma once
#include "player_state.h"
#include <SFML/Graphics.hpp>

class Player;

class LandingState : public PlayerState {
  public:
	const sf::Texture jump_texture{"./assets/images/player/jump.png"};

	static constexpr float LAND_FRAME_DURATION = 0.07f;

	PlayerState *update(float dt, Player &p) override;
	void applyAnimation(float dt, Player &p) override;
	void onEnter(Player &p) override;

  private:
	bool animationComplete = false;
};
