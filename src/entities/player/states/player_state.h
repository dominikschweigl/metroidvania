#pragma once
#include <SFML/Graphics.hpp>

class Player; // forward declaration — method bodies only need the pointer/ref type

class PlayerState {
  public:
	virtual ~PlayerState() = default;

	// Returns next state (return `this` for no transition).
	[[nodiscard]] virtual PlayerState *update(float dt, Player &p) = 0;

	virtual void applyAnimation(float dt, Player &p) = 0;

	// Override to allow attacks in a specific state.
	virtual bool canAttack() const noexcept { return false; }

	virtual void onEnter(Player &p) {}
	virtual void onExit(Player &p) {}

  protected:
	int currentFrame = 0;
	float frameTimer = 0.f;
};
