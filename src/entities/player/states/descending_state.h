#pragma once
#include "../../../core/asset_manager.h"
#include "player_state.h"

class Player;

class DescendingState : public PlayerState {
  public:
	const sf::Texture &jump_texture;

	static constexpr sf::Vector2f HEAD_OFFSET = sf::Vector2f{0.f, 0.f};

	DescendingState();

	sf::Vector2f getHeadOffset() const noexcept override;
	PlayerState *update(float dt, Player &p) override;
	void applyAnimation(float dt, Player &p) override;
	void onEnter(Player &p) override;
};
