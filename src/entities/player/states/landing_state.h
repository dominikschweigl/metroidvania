#pragma once
#include "../../../core/asset_manager.h"
#include "player_state.h"
#include <array>

class Player;

class LandingState : public PlayerState {
  public:
	const sf::Texture &jump_texture;

	LandingState();

	static constexpr float LAND_FRAME_DURATION = 0.07f;

	static constexpr std::array<sf::Vector2f, 4> HEAD_OFFSETS = {
	    sf::Vector2f{1.f, 2.f}, {1.f, 3.f}, {1.f, 4.f}, {1.f, 2.f}};

	sf::Vector2f getHeadOffset() const noexcept override;
	PlayerState *update(float dt, Player &p) override;
	void applyAnimation(float dt, Player &p) override;
	void onEnter(Player &p) override;

  private:
	bool animationComplete = false;
};
