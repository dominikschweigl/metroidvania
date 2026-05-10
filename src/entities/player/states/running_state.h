#pragma once
#include "../../../core/asset_manager.h"
#include "player_state.h"

class Player;

class RunningState : public PlayerState {
  public:
	const sf::Texture &run_texture;
	const sf::Texture &run_lower_texture;
	const sf::Texture &run_upper_texture;

	RunningState();

	static constexpr float WALK_FRAME_DURATION = 1 / 10.f;

	bool canAttack() const noexcept override { return true; }
	PlayerState *update(float dt, Player &p) override;
	void applyAnimation(float dt, Player &p) override;
	void onEnter(Player &p) override;
};
