#pragma once
#include <SFML/Graphics.hpp>

#include "../../world/world.h"
#include "attack_layer.h"
#include "states/ascending_state.h"
#include "states/descending_state.h"
#include "states/idle_state.h"
#include "states/landing_state.h"
#include "states/peak_state.h"
#include "states/pre_jump_state.h"
#include "states/running_state.h"
#include "states/walking_state.h"

class Player {
  public:
	enum class Direction { Left, Right };
	Direction direction = Direction::Left;

	static constexpr float GRAVITY = 1200.f;
	static constexpr float WALKING_SPEED = 200.f;
	static constexpr float RUNNING_SPEED = 350.f;
	static constexpr float JUMP_SPEED = 500.f;

	static constexpr int FRAME_SIZE = 32;
	static constexpr float PEAK_THRESHOLD = 250.f;

	bool debugHorizontalMovement = false;
	sf::RectangleShape debugHorizontalCollisionCheck;
	bool debugVerticalMovement = false;
	sf::RectangleShape debugVerticalCollisionCheck;

	Player();
	~Player() = default;

	sf::FloatRect getBounds() const
	{
		return sf::Rect<float>({sprite.getPosition().x - FRAME_SIZE / 2.f, sprite.getPosition().y - FRAME_SIZE},
		                       {FRAME_SIZE, FRAME_SIZE});
	}

	void update(float deltaTime, const World *world = nullptr, bool attackTriggered = false);

	void draw(sf::RenderWindow &window);

	sf::Vector2f getPosition() const { return sprite.getPosition(); }

  private:
	bool isOnGround = true;
	bool inputJump = false;
	sf::Vector2f velocity;

	AttackLayer attackLayer;

	// State pool — stored by value, transitions are pointer swaps (no allocation).
	// Must be declared before sprite/upperSprite so textures are ready for the constructor initializer.
	struct States {
		IdleState idle;
		WalkingState walking;
		RunningState running;
		PreJumpState preJump;
		AscendingState ascending;
		PeakState peak;
		DescendingState descending;
		LandingState landing;
	} states;

	sf::Sprite sprite;
	sf::Sprite upperSprite;

	PlayerState *currentState;

	void transitionTo(PlayerState &next)
	{
		currentState->onExit(*this);
		next.onEnter(*this);
		currentState = &next;
		if (!currentState->canAttack())
			attackLayer.reset();
	}

	bool isGroundBelow(const World &world) const
	{
		auto leftTile =
		    world.getTileAtCoordinate({getBounds().position.x, getBounds().position.y + getBounds().size.y + 1.f});
		auto rightTile = world.getTileAtCoordinate(
		    {getBounds().position.x + getBounds().size.x, getBounds().position.y + getBounds().size.y + 1.f});
		if (!leftTile.has_value() || !rightTile.has_value())
			return false;
		return (leftTile.value()->isSolid || rightTile.value()->isSolid);
	}

	float handleHorizontalMovement(const World &world, float deltaTime)
	{
		float dx = velocity.x * deltaTime;
		float futureX = sprite.getPosition().x + dx;

		auto bounds = getBounds();
		auto futureBounds = sf::FloatRect({futureX - FRAME_SIZE / 2.f, bounds.position.y}, {FRAME_SIZE, bounds.size.y});

		if (debugHorizontalMovement) {
			debugHorizontalCollisionCheck = sf::RectangleShape(futureBounds.size);
			debugHorizontalCollisionCheck.setPosition(futureBounds.position);
			debugHorizontalCollisionCheck.setFillColor(sf::Color(0, 255, 0, 100));
		}

		if (world.isSolidAtRect(futureBounds)) {
			velocity.x = 0.f;
			std::optional<const World::Tile *> tile = world.getTileAtCoordinate(sprite.getPosition());
			if (tile.has_value()) {
				if (dx > 0)
					futureX = tile.value()->position.x + World::TILE_SIZE - FRAME_SIZE / 2.f - 1.f;
				else if (dx < 0)
					futureX = tile.value()->position.x + FRAME_SIZE / 2.f;
			}
		}

		return futureX;
	}

	float handleVerticalMovement(const World &world, float deltaTime)
	{
		float dy = velocity.y * deltaTime;
		float futureY = sprite.getPosition().y + dy;

		auto bounds = getBounds();
		auto futureBounds = sf::FloatRect({bounds.position.x, futureY - bounds.size.y}, {FRAME_SIZE, FRAME_SIZE});

		if (debugVerticalMovement) {
			debugVerticalCollisionCheck = sf::RectangleShape(futureBounds.size);
			debugVerticalCollisionCheck.setPosition(futureBounds.position);
			debugVerticalCollisionCheck.setFillColor(sf::Color(255, 0, 0, 100));
		}

		if (world.isSolidAtRect(futureBounds)) {
			velocity.y = 0.f;
			auto tile = world.getTileAtCoordinate(futureBounds.position);
			if (tile.has_value()) {
				if (dy > 0) {
					futureY = tile.value()->position.y + World::TILE_SIZE;
					isOnGround = true;
					transitionTo(states.landing);
				} else if (dy < 0) {
					futureY = tile.value()->position.y + World::TILE_SIZE + FRAME_SIZE;
				}
			}
		}
		return futureY;
	}

	void handleMovement(float deltaTime, const World *world = nullptr);

	void updateAnimation(float dt);

	friend class IdleState;
	friend class WalkingState;
	friend class RunningState;
	friend class PreJumpState;
	friend class AscendingState;
	friend class PeakState;
	friend class DescendingState;
	friend class LandingState;
};
