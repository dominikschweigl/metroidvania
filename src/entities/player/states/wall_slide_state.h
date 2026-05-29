#pragma once
#include "../../../core/asset_manager.h"
#include "../../direction.h"
#include "player_state.h"

class Player;

class WallSlideState : public PlayerState {
  public:
	const sf::Texture &wall_slide_lower_texture;
	const sf::Texture &wall_slide_upper_texture;

	static constexpr float WALL_SLIDE_GRAVITY_FACTOR = 0.15f;
	static constexpr sf::Vector2f HEAD_OFFSET = sf::Vector2f{0.f, 0.f};
	static constexpr sf::Vector2f UPPER_BODY_OFFSET = sf::Vector2f{0.f, 0.f};

	WallSlideState();

	sf::Vector2f getHeadOffset() const noexcept override;
	sf::Vector2f getUpperBodyOffset() const noexcept override;
	PlayerState *update(float dt, Player &p) override;
	void applyAnimation(float dt, Player &p) override;
	void onEnter(Player &p) override;
	void onExit(Player &p) override;

  private:
	float originalGravity = 0.f;
	Direction wallDirection = Direction::Left;
};
