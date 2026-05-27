#pragma once
#include <SFML/Graphics.hpp>

#include "../../combat/health.h"
#include "../../combat/hitbox.h"
#include "../../core/direction.h"
#include "../../world/world.h"
#include "hat_ability.h"
#include "melee_attack.h"
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

	static constexpr int MAX_HEALTH = 5;

	// Duration of initial invincibility for better combat feel
	static constexpr float IFRAME_DURATION = 0.5f;

	Health health{MAX_HEALTH, MAX_HEALTH};

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

	[[nodiscard]] bool isAlive() const noexcept { return health.isAlive(); }
	[[nodiscard]] float getIframes() const noexcept { return iframes; }
	[[nodiscard]] Hurtbox getHurtbox() noexcept { return Hurtbox{getBounds(), Team::Player, &health, iframes > 0.f}; }

  private:
	bool isOnGround = true;
	bool inputJump = false;
	bool isSprinting = false;
	sf::Vector2f velocity;

	float iframes = 0.f;
	int previousHealth = MAX_HEALTH;

	MeleeAttack meleeAttack;
	HatAbility hatAbility;

	States states;

	sf::Sprite lowerBodySprite;
	sf::Sprite headSprite;
	sf::Sprite upperBodySprite;

	PlayerState *currentState;

	void transitionTo(PlayerState &next);

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
