#pragma once
#include "../../../core/asset_manager.h"
#include "player_state.h"

class Player;

class DescendingState : public PlayerState {
  public:
	const sf::Texture &jump_lower_texture;
	const sf::Texture &jump_upper_texture;

	static constexpr sf::Vector2f HEAD_OFFSET = sf::Vector2f{0.f, 0.f};
	static constexpr sf::Vector2f UPPER_BODY_OFFSET = sf::Vector2f{0.f, 0.f};
	static constexpr sf::Vector2f ATTACK_UPPER_BODY_OFFSET = sf::Vector2f{0.f, 0.f};

	DescendingState();

	bool canAttack() const noexcept override { return true; }
	sf::Vector2f getHeadOffset(Player &p) const noexcept override;
	sf::Vector2f getUpperBodyOffset(Player &p) const noexcept override;
	PlayerState *update(float dt, Player &p) override;
	void applyAnimation(float dt, Player &p) override;
	void onEnter(Player &p) override;
};
