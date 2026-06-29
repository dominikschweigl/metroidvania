#pragma once
#include <SFML/Graphics.hpp>

class Player; // forward declaration - method bodies only need the pointer/ref type

class PlayerState {
  public:
	virtual ~PlayerState() = default;

	// Returns next state (return `this` for no transition).
	[[nodiscard]] virtual PlayerState *update(float dt, Player &p) = 0;

	virtual void applyAnimation(float dt, Player &p) = 0;

	// Override to allow attacks in a specific state.
	virtual bool canAttack() const noexcept { return false; }

	virtual sf::Vector2f getHeadOffset(Player &) const noexcept { return {0.f, 0.f}; }
	virtual sf::Vector2f getUpperBodyOffset(Player &) const noexcept { return {0.f, 0.f}; }

	virtual void onEnter(Player &) {}
	virtual void onExit(Player &) {}

  protected:
	int currentFrame = 0;
	float frameTimer = 0.f;
};
