#pragma once
#include <SFML/Graphics.hpp>

#include "../../combat/hitbox.h"
#include "../base_entity.h"
#include "abilities/hat_ability.h"
#include "abilities/melee_attack.h"
#include "states/ascending_state.h"
#include "states/descending_state.h"
#include "states/idle_state.h"
#include "states/landing_state.h"
#include "states/peak_state.h"
#include "states/player_state.h"
#include "states/pre_jump_state.h"
#include "states/running_state.h"
#include "states/walking_state.h"
#include "states/wall_slide_state.h"
#include <vector>

class World;

class Player : public BaseEntity {
  public:
	struct States {
		IdleState idle;
		WalkingState walking;
		RunningState running;
		PreJumpState preJump;
		AscendingState ascending;
		PeakState peak;
		DescendingState descending;
		LandingState landing;
		WallSlideState wallSlide;
	};

	static constexpr float WALKING_SPEED = 200.f;
	static constexpr float RUNNING_SPEED = 350.f;
	static constexpr float JUMP_SPEED = 500.f;

	static constexpr int FRAME_SIZE = 32;
	static constexpr float PEAK_THRESHOLD = 250.f;

	static constexpr int MAX_HEALTH = 5;

	// Duration of initial invincibility for better combat feel
	static constexpr float IFRAME_DURATION = 0.5f;
	static constexpr float WALL_JUMP_DURATION = 0.35f;

	bool debugHorizontalMovement = false;
	sf::RectangleShape debugHorizontalCollisionCheck;
	bool debugVerticalMovement = false;
	sf::RectangleShape debugVerticalCollisionCheck;

	Player();
	~Player() override = default;

	void update(float deltaTime, const World &world, bool attackTriggered = false, bool hatThrowTriggered = false);

	void draw(sf::RenderWindow &window);

	[[nodiscard]] bool isPlayerOnGround() const noexcept { return isOnGroundFlag(); }

	[[nodiscard]] bool isAttackActive() const noexcept
	{
		return meleeAttack.isMeleeActive() || hatAbility.isThrowActive();
	}
	[[nodiscard]] bool hasHatThrown() const noexcept { return hatAbility.hasProjectile(); }
	[[nodiscard]] HatProjectile &getThrownHat() noexcept { return hatAbility.getProjectile(); }

	[[nodiscard]] float getIframes() const noexcept { return iframes; }

	[[nodiscard]] bool isInvulnerable() const noexcept override { return iframes > 0.f; }

	[[nodiscard]] std::optional<Hitbox> getMeleeHitbox() const noexcept
	{
		return meleeAttack.getHitbox(position, getDirection());
	}

	void collectHitboxes(std::vector<Hitbox> &hitboxes) override;
	void drainEndedSourceIds(std::vector<std::uint32_t> &out) override;

  private:
	bool inputJump = false;
	bool inputLeft = false;
	bool inputRight = false;
	bool isSprinting = false;
	bool isAgainstLeftWall = false;
	bool isAgainstRightWall = false;
	float wallJumpTimer = 0.f;

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
	friend class WallSlideState;

	friend struct PlayerTestAccess;
};
