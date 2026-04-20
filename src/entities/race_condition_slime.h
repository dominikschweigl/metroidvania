#pragma once
#include "../world/world.h"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <random>

// Race Condition based slime enemy
// States: Idle -> Chase -> WindUp -> Attack -> Recover -> Chase/Idle
class RaceConditionSlime {
  public:
	static constexpr float GRAVITY = 1200.f;
	static constexpr float MOVE_SPEED = 150.f;
	static constexpr float MAX_JUMP_SPEED = 700.f;

	static constexpr float DETECT_RANGE = 420.f;
	static constexpr float ATTACK_RANGE = 28.f;
	static constexpr float LOSE_RANGE = 650.f;

	static constexpr float WINDUP_DUR = 0.4f;
	static constexpr float ATTACK_DUR = 0.2f;
	static constexpr float RECOVER_DUR = 0.5f;

	static constexpr float JUMP_THRESHOLD = 40.f; // player must be this much higher
	static constexpr float JUMP_COOLDOWN = 1.2f;
	static constexpr float ATTACK_COOLDOWN = 5.f; // time between consecutive attack starts

	static constexpr float ENTITY_WIDTH = 28.f;
	static constexpr float ENTITY_HEIGHT = 28.f;
	static constexpr int FRAME_SIZE = 32;

	enum class Direction { Left, Right };

	explicit RaceConditionSlime(sf::Vector2f spawnPos) : pos(spawnPos), rng(std::random_device{}())
	{
		if (idleTexture.loadFromFile("./assets/images/enemies/race_condition/idle.png")) {
			sprite.setTexture(idleTexture);
		}
		(void)movingTexture.loadFromFile("./assets/images/enemies/race_condition/moving.png");
		(void)attackTexture.loadFromFile("./assets/images/enemies/race_condition/attack.png");
		sprite.setOrigin({FRAME_SIZE / 2.f, static_cast<float>(FRAME_SIZE)});
		resetTeleportTimer();
	}

	sf::FloatRect getBounds() const
	{
		return {{pos.x - ENTITY_WIDTH / 2.f, pos.y - ENTITY_HEIGHT}, {ENTITY_WIDTH, ENTITY_HEIGHT}};
	}

	sf::Vector2f getPosition() const { return pos; }

	bool isAttacking() const { return state == State::Attack; }

	void update(float deltaTime, const World &world, sf::Vector2f playerPos)
	{
		stateTimer += deltaTime;
		teleportTimer -= deltaTime;
		jumpCooldown -= deltaTime;
		attackCooldown -= deltaTime;

		float deltaX = playerPos.x - pos.x;
		float heightDiff = pos.y - playerPos.y; // positive when player is higher
		float dist = std::abs(deltaX);
		facing = (deltaX >= 0.f) ? Direction::Right : Direction::Left;

		switch (state) {
		case State::Idle: {
			vel.x = 0.f;
			// Enemy should stay in cooldown state in between attacks
			bool waitingForCooldown = attackCooldown > 0.f && dist < ATTACK_RANGE;
			if (dist < DETECT_RANGE && !waitingForCooldown)
				transitionTo(State::Chase);
			break;
		}

		case State::Chase:
			// Only strike when within melee reach on x and y axes to avoid change to WindUp state
			if (dist < ATTACK_RANGE && std::abs(heightDiff) < ATTACK_RANGE) {
				vel.x = 0.f;
				transitionTo(attackCooldown <= 0.f ? State::WindUp : State::Idle);
				break;
			}
			if (dist > LOSE_RANGE) {
				vel.x = 0.f;
				transitionTo(State::Idle);
				break;
			}
			vel.x = (facing == Direction::Right ? 1.f : -1.f) * MOVE_SPEED;

			// Reach higher platforms by jumping if player is above and jump isn't on cooldown.
			if (isOnGround && heightDiff > JUMP_THRESHOLD && jumpCooldown <= 0.f) {
				float necessaryVelocity = std::sqrt(2.f * GRAVITY * (heightDiff + ENTITY_HEIGHT));
				vel.y = -std::min(necessaryVelocity, MAX_JUMP_SPEED);
				isOnGround = false;
				jumpCooldown = JUMP_COOLDOWN;
			}

			if (teleportTimer <= 0.f) {
				glitchTeleport(world, playerPos);
			}

			break;

		case State::WindUp:
			vel.x = 0.f;
			if (stateTimer >= WINDUP_DUR) {
				transitionTo(State::Attack);
			}

			break;

		case State::Attack:
			vel.x = 0.f; // stationary strike from the standoff position
			if (stateTimer >= ATTACK_DUR) {
				transitionTo(State::Recover);
			}

			break;

		case State::Recover:
			vel.x = 0.f;
			if (stateTimer >= RECOVER_DUR) {
				transitionTo(dist < DETECT_RANGE ? State::Chase : State::Idle);
			}

			break;
		}

		if (!isGroundBelow(world)) {
			isOnGround = false;
			vel.y += GRAVITY * deltaTime;
		}

		pos.x = resolveHorizontal(deltaTime, world);
		pos.y = resolveVertical(deltaTime, world);

		updateAnimation(deltaTime);
	}

	void draw(sf::RenderWindow &window)
	{
		sprite.setPosition(pos);
		sprite.setScale({facing == Direction::Right ? 1.f : -1.f, 1.f});
		window.draw(sprite);
	}

  private:
	enum class State { Idle, Chase, WindUp, Attack, Recover };

	sf::Vector2f pos;
	sf::Vector2f vel{0.f, 0.f};
	bool isOnGround = false;

	State state = State::Idle;
	State animState = State::Idle;
	Direction facing = Direction::Right;

	float stateTimer = 0.f;
	float teleportTimer = 0.f;
	float jumpCooldown = 0.f;
	float attackCooldown = 0.f;

	sf::Texture idleTexture;
	sf::Texture movingTexture;
	sf::Texture attackTexture;
	sf::Sprite sprite{idleTexture};
	int currentFrame = 0;
	float frameTimer = 0.f;

	std::mt19937 rng;
	static constexpr float IDLE_FRAME_DURATION = 0.1f;
	static constexpr int IDLE_FRAME_COUNT = 8;
	static constexpr float MOVING_FRAME_DURATION = 0.08f;
	static constexpr int MOVING_FRAME_COUNT = 8;
	static constexpr int ATTACK_FRAME_COUNT = 12;
	static constexpr float ATTACK_FRAME_DURATION = (WINDUP_DUR + ATTACK_DUR + RECOVER_DUR) / ATTACK_FRAME_COUNT;

	float uniformFloat(float lo, float hi) { return std::uniform_real_distribution<float>(lo, hi)(rng); }

	void resetTeleportTimer() { teleportTimer = uniformFloat(1.0f, 2.5f); }

	void transitionTo(State next)
	{
		state = next;
		stateTimer = 0.f;
		if (next == State::WindUp) {
			attackCooldown = ATTACK_COOLDOWN;
			animState = State::WindUp;
			currentFrame = 0;
			frameTimer = 0.f;
		}
	}

	void updateAnimation(float deltaTime)
	{
		int frameCount;
		float frameDuration;
		switch (animState) {
		case State::Chase:
			frameCount = MOVING_FRAME_COUNT;
			frameDuration = MOVING_FRAME_DURATION;
			break;
		case State::WindUp: // single animation covers the whole WindUp/Attack/Recover sequence
			frameCount = ATTACK_FRAME_COUNT;
			frameDuration = ATTACK_FRAME_DURATION;
			break;
		default:
			frameCount = IDLE_FRAME_COUNT;
			frameDuration = IDLE_FRAME_DURATION;
			break;
		}

		frameTimer += deltaTime;
		if (frameTimer >= frameDuration) {
			frameTimer -= static_cast<int>(frameTimer / frameDuration) * frameDuration;
			currentFrame++;
			if (currentFrame >= frameCount) {
				// Cycle complete — now safe to swap to the animation matching current logical state.
				currentFrame = 0;
				State desiredAnim = State::Idle;
				if (state == State::Chase)
					desiredAnim = State::Chase;
				else if (state == State::WindUp || state == State::Attack || state == State::Recover)
					desiredAnim = State::WindUp;
				if (desiredAnim != animState) {
					animState = desiredAnim;
					frameTimer = 0.f;
				}
			}
		}

		const sf::Texture *texture = &idleTexture;
		if (animState == State::Chase)
			texture = &movingTexture;
		else if (animState == State::WindUp)
			texture = &attackTexture;

		sprite.setTexture(*texture);
		sprite.setTextureRect(sf::IntRect({currentFrame * FRAME_SIZE, 0}, {FRAME_SIZE, FRAME_SIZE}));
	}

	void glitchTeleport(const World &world, sf::Vector2f playerPos)
	{
		resetTeleportTimer();
		float dir = (playerPos.x >= pos.x) ? 1.f : -1.f;

		// Up to 8 attempts to find a landing spot that is neither inside a
		// wall nor over a pit too deep to jump back out of.
		for (int attempt = 0; attempt < 8; ++attempt) {
			switch (std::uniform_int_distribution<int>(0, 2)(rng)) {
			case 0: { // forward
				float newX = pos.x + dir * uniformFloat(80.f, 220.f);
				if (isValidTeleportDest(world, newX, pos.y)) {
					pos.x = newX;
					return;
				}
				break;
			}
			case 1: { // backward
				float newX = pos.x - dir * uniformFloat(60.f, 160.f);
				if (isValidTeleportDest(world, newX, pos.y)) {
					pos.x = newX;
					return;
				}
				break;
			}
			case 2: // upward — in-place velocity boost, always safe
				vel.y = -uniformFloat(380.f, 580.f);
				isOnGround = false;
				return;
			}
		}
	}

	// Reject destinations that overlap solid geometry or sit over a drop too
	// deep to climb back out of (kept within jump reach on purpose).
	bool isValidTeleportDest(const World &world, float newX, float newY) const
	{
		sf::FloatRect dest({newX - ENTITY_WIDTH / 2.f, newY - ENTITY_HEIGHT}, {ENTITY_WIDTH, ENTITY_HEIGHT});
		if (world.isSolidAtRect(dest))
			return false;

		constexpr float MAX_FALL = 5.f * World::TILE_SIZE; // stays within jump reach
		for (float delta = 1.f; delta <= MAX_FALL; delta += World::TILE_SIZE / 2.f) {
			float y = newY + delta;
			auto lhs = world.getTileAtCoordinate({newX - ENTITY_WIDTH / 2.f, y});
			auto rhs = world.getTileAtCoordinate({newX + ENTITY_WIDTH / 2.f, y});
			if ((lhs.has_value() && lhs.value()->isSolid) || (rhs.has_value() && rhs.value()->isSolid))
				return true;
		}
		return false;
	}

	bool isGroundBelow(const World &world) const
	{
		auto b = getBounds();
		float bott = b.position.y + b.size.y;
		auto left = world.getTileAtCoordinate({b.position.x, bott + 1.f});
		auto right = world.getTileAtCoordinate({b.position.x + b.size.x, bott + 1.f});
		return (left.has_value() && left.value()->isSolid) || (right.has_value() && right.value()->isSolid);
	}

	float resolveHorizontal(float deltaTime, const World &world)
	{
		float deltaX = vel.x * deltaTime;
		float futureX = pos.x + deltaX;
		auto bounds = getBounds();

		sf::FloatRect future({futureX - ENTITY_WIDTH / 2.f, bounds.position.y}, {ENTITY_WIDTH, ENTITY_HEIGHT});
		if (world.isSolidAtRect(future)) {
			vel.x = 0.f;
			auto tile = world.getTileAtCoordinate(pos);
			if (tile.has_value()) {
				if (deltaX >= 0.f) {
					return tile.value()->position.x + World::TILE_SIZE - ENTITY_WIDTH / 2.f - 1.f;
				} else {
					return tile.value()->position.x + ENTITY_WIDTH / 2.f;
				}
			}
			return pos.x;
		}
		return futureX;
	}

	float resolveVertical(float deltaTime, const World &world)
	{
		float deltaY = vel.y * deltaTime;
		float futureY = pos.y + deltaY;
		auto bounds = getBounds();

		sf::FloatRect future({bounds.position.x, futureY - ENTITY_HEIGHT}, {ENTITY_WIDTH, ENTITY_HEIGHT});
		if (world.isSolidAtRect(future)) {
			auto tile = world.getTileAtCoordinate(future.position);
			if (tile.has_value()) {
				if (deltaY > 0.f) {
					futureY = tile.value()->position.y + World::TILE_SIZE;
					isOnGround = true;
				} else if (deltaY < 0.f) {
					futureY = tile.value()->position.y + World::TILE_SIZE + ENTITY_HEIGHT;
				}
			}
			vel.y = 0.f;
		}
		return futureY;
	}
};
