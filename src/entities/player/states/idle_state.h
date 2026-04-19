#pragma once
#include "player_state.h"
#include <SFML/Graphics.hpp>

class Player;

class IdleState : public PlayerState {
  public:
	const sf::Texture idle_texture{"./assets/images/player/idle_hat.png"};
	const sf::Texture idle_lower_texture{"./assets/images/player/idle_lower_body_extended.png"};

	bool canAttack() const noexcept override { return true; }
	PlayerState *update(float dt, Player &p) override;
	void applyAnimation(float dt, Player &p) override;
	void onEnter(Player &p) override;
};
