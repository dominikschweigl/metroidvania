#pragma once
#include "../../../core/asset_manager.h"
#include "player_state.h"
#include <array>

class Player;

class RunningState : public PlayerState {
  public:
	const sf::Texture &run_lower_texture;
	const sf::Texture &run_upper_texture;

	RunningState();

	static constexpr float WALK_FRAME_DURATION = 1 / 10.f;
	static constexpr float STEP_INTERVAL = 0.25f;
	static constexpr float STEP_VOLUME = 20.f;

	static constexpr std::array<sf::Vector2f, 8> HEAD_OFFSETS = {
	    sf::Vector2f{0.f, 0.f}, {1.f, 0.f}, {1.f, 0.f}, {1.f, 0.f}, {1.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}};

	bool canAttack() const noexcept override { return true; }
	sf::Vector2f getHeadOffset(Player &p) const noexcept override;
	PlayerState *update(float dt, Player &p) override;
	void applyAnimation(float dt, Player &p) override;
	void onEnter(Player &p) override;

  private:
	float stepTimer = 0.f;
	int nextStepIndex = 0;

	void triggerStep();
};
