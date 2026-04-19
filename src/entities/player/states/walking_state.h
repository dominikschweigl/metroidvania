#pragma once
#include "player_state.h"
#include <SFML/Graphics.hpp>

class Player;

class WalkingState : public PlayerState {
  public:
	const sf::Texture walk_texture{"./assets/images/player/walk.png"};
	const sf::Texture walk_lower_texture{"./assets/images/player/walk_lower_body_extended.png"};
	const sf::Texture walk_upper_texture{"./assets/images/player/walk_upper_body.png"};

	static constexpr float WALK_FRAME_DURATION = 1 / 10.f;

	bool canAttack() const noexcept override { return true; }
	PlayerState *update(float dt, Player &p) override;
	void applyAnimation(float dt, Player &p) override;
	void onEnter(Player &p) override;
};
