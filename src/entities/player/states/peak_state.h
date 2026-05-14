#pragma once
#include "../../../core/asset_manager.h"
#include "player_state.h"

class Player;

class PeakState : public PlayerState {
  public:
	const sf::Texture &jump_lower_texture;
	const sf::Texture &jump_upper_texture;

	static constexpr sf::Vector2f HEAD_OFFSET = sf::Vector2f{0.f, -1.f};
	static constexpr sf::Vector2f UPPER_BODY_OFFSET = sf::Vector2f{0.f, 0.f};

	PeakState();

	sf::Vector2f getHeadOffset() const noexcept override;
	sf::Vector2f getUpperBodyOffset() const noexcept override;
	PlayerState *update(float dt, Player &p) override;
	void applyAnimation(float dt, Player &p) override;
	void onEnter(Player &p) override;
};
