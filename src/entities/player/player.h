#pragma once
#include <SFML/Graphics.hpp>
#include <optional>

#include "../../core/direction.h"
#include "../../world/world.h"
#include "../base/entity_physics.h"
#include "melee_attack.h"
#include "hat_ability.h"
#include "states/ascending_state.h"
#include "states/descending_state.h"
#include "states/idle_state.h"
#include "states/landing_state.h"
#include "states/peak_state.h"
#include "states/player_state.h"
#include "states/pre_jump_state.h"
#include "states/running_state.h"
#include "states/walking_state.h"

class Player {
  public:
	Direction direction = Direction::Left;

	struct States {
		IdleState idle;
		WalkingState walking;
		RunningState running;
		PreJumpState preJump;
		AscendingState ascending;
		PeakState peak;
		DescendingState descending;
		LandingState landing;
	};

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
		return sf::Rect<float>(
		    {lowerBodySprite.getPosition().x - FRAME_SIZE / 2.f, lowerBodySprite.getPosition().y - FRAME_SIZE},
		    {FRAME_SIZE, FRAME_SIZE});
	}

	void update(float deltaTime, const World &world, bool attackTriggered = false, bool hatThrowTriggered = false);

	void draw(sf::RenderWindow &window);

	sf::Vector2f getPosition() const { return lowerBodySprite.getPosition(); }

	[[nodiscard]] bool isAttackActive() const noexcept
	{
		return meleeAttack.isMeleeActive() || hatAbility.isThrowActive();
	}
	[[nodiscard]] bool hasHatThrown() const noexcept { return hatAbility.hasProjectile(); }
	[[nodiscard]] HatProjectile &getThrownHat() noexcept { return hatAbility.getProjectile(); }

  private:
	bool isOnGround = true;
	bool inputJump = false;
	bool isSprinting = false;
	sf::Vector2f velocity;

	MeleeAttack meleeAttack;
	HatAbility hatAbility;

	States states;

	sf::Sprite lowerBodySprite;
	sf::Sprite headSprite;
	sf::Sprite upperBodySprite;

	PlayerState *currentState;

	void transitionTo(PlayerState &next)
	{
		currentState->onExit(*this);
		next.onEnter(*this);
		currentState = &next;
		if (!currentState->canAttack()) {
			meleeAttack.reset();
			hatAbility.reset();
		}
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
		float futureX = lowerBodySprite.getPosition().x + dx;

		auto bounds = getBounds();
		auto futureBounds = sf::FloatRect({futureX - FRAME_SIZE / 2.f, bounds.position.y}, {FRAME_SIZE, bounds.size.y});

		if (debugHorizontalMovement) {
			debugHorizontalCollisionCheck = sf::RectangleShape(futureBounds.size);
			debugHorizontalCollisionCheck.setPosition(futureBounds.position);
			debugHorizontalCollisionCheck.setFillColor(sf::Color(0, 255, 0, 100));
		}

		if (world.isSolidAtRect(futureBounds)) {
			velocity.x = 0.f;
			std::optional<const World::Tile *> tile = world.getTileAtCoordinate(lowerBodySprite.getPosition());
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
		float futureY = lowerBodySprite.getPosition().y + dy;

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

	void handleMovement(float deltaTime, const World &world);

	void updateAnimation(float dt);

	friend class IdleState;
	friend class WalkingState;
	friend class RunningState;
	friend class PreJumpState;
	friend class AscendingState;
	friend class PeakState;
	friend class DescendingState;
	friend class LandingState;

	friend struct PlayerTestAccess;
};
