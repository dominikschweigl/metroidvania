#pragma once
#include "../../../core/asset_manager.h"
#include "player_state.h"

class Player;

class AscendingState : public PlayerState {
  public:
	const sf::Texture &jump_texture;

	static constexpr sf::Vector2f HEAD_OFFSET = sf::Vector2f{1.f, -1.f};

	AscendingState();

	sf::Vector2f getHeadOffset() const noexcept override;
	PlayerState *update(float dt, Player &p) override;
	void applyAnimation(float dt, Player &p) override;
	void onEnter(Player &p) override;
};
