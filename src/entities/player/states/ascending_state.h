#pragma once
#include "player_state.h"
#include <SFML/Graphics.hpp>

class Player;

class AscendingState : public PlayerState {
  public:
	const sf::Texture jump_texture{"./assets/images/player/jump.png"};

	PlayerState *update(float dt, Player &p) override;
	void applyAnimation(float dt, Player &p) override;
	void onEnter(Player &p) override;
};
