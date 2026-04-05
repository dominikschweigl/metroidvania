#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class Player {
  public:
	enum class Direction { Left, Right };
	Direction direction = Direction::Left;

	static constexpr float GRAVITY = 1200.f;
	static constexpr float SPEED = 200.f;
	static constexpr float JUMP_SPEED = 500.f;

	static constexpr int FRAME_SIZE = 32;
	static constexpr float WALK_FRAME_DURATION = 1 / 10.f;
	static constexpr float PREJUMP_FRAME_DURATION = 0.08f;
	static constexpr float LAND_FRAME_DURATION = 0.07f;
	static constexpr float PEAK_THRESHOLD = 250.f; // velocity.y range considered "peak"

	const sf::Texture idle_texture{"./assets/images/player/idle_hat.png"};
	const sf::Texture walk_texture{"./assets/images/player/walk.png"};
	const sf::Texture jump_texture{"./assets/images/player/jump.png"};

	sf::Sprite sprite;

	bool isOnGround = true;
	sf::Vector2f velocity;

	Player() : sprite(idle_texture) { sprite.setOrigin({FRAME_SIZE / 2.f, static_cast<float>(FRAME_SIZE)}); }
	~Player() = default;

	void update(float deltaTime)
	{
		handleMovement(deltaTime);
		updateAnimation(deltaTime);
	}

	sf::Vector2f getPosition() const { return sprite.getPosition(); }

  private:
	enum class JumpState { None, PreJump, Ascending, Peak, Descending, Landing };
	JumpState jumpState = JumpState::None;

	int currentFrame = 0;
	float frameTimer = 0.f;

	void handleMovement(float deltaTime)
	{
		velocity.x = 0.f;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
			velocity.x = -SPEED;
			direction = Direction::Left;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
			velocity.x = SPEED;
			direction = Direction::Right;
		}

		bool jumpPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);

		if (isOnGround && jumpPressed && jumpState == JumpState::None) {
			jumpState = JumpState::PreJump;
			currentFrame = 0;
			frameTimer = 0.f;
		}

		// Delay takeoff until pre-jump animation finishes
		if (jumpState != JumpState::PreJump) {
			if (!isOnGround) {
				velocity.y += GRAVITY * deltaTime;
			}

			sprite.move(velocity * deltaTime);

			if (sprite.getPosition().y >= 0.f) {
				sprite.setPosition({sprite.getPosition().x, 0.f});
				velocity.y = 0.f;
				if (!isOnGround) {
					isOnGround = true;
					if (jumpState != JumpState::None) {
						jumpState = JumpState::Landing;
						currentFrame = 0;
						frameTimer = 0.f;
					}
				}
			}
		}
	}

	void updateAnimation(float deltaTime)
	{
		bool isWalking = velocity.x != 0.f;
		bool isJumping = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);

		// Interrupt landing with walk or jump input
		if (jumpState == JumpState::Landing && isJumping) {
			jumpState = JumpState::PreJump;
			currentFrame = 0;
		}

		switch (jumpState) {
		case JumpState::None:
			if (isWalking) {
				sprite.setTexture(walk_texture);
				frameTimer += deltaTime;
				if (frameTimer >= WALK_FRAME_DURATION) {
					frameTimer -= WALK_FRAME_DURATION;
					currentFrame = (currentFrame + 1) % 16;
				}
				sprite.setTextureRect(sf::IntRect({currentFrame * FRAME_SIZE, 0}, {FRAME_SIZE, FRAME_SIZE}));
			} else {
				sprite.setTexture(idle_texture);
				sprite.setTextureRect(sf::IntRect({0, 0}, {FRAME_SIZE, FRAME_SIZE}));
				currentFrame = 0;
				frameTimer = 0.f;
			}
			break;

		case JumpState::PreJump:
			sprite.setTexture(jump_texture);
			sprite.setTextureRect(sf::IntRect({currentFrame * FRAME_SIZE, 0}, {FRAME_SIZE, FRAME_SIZE}));
			frameTimer += deltaTime;
			if (frameTimer >= PREJUMP_FRAME_DURATION) {
				frameTimer -= PREJUMP_FRAME_DURATION;
				currentFrame++;
				if (currentFrame >= 2) {
					// Launch
					velocity.y = -JUMP_SPEED;
					isOnGround = false;
					jumpState = JumpState::Ascending;
					currentFrame = 2;
				}
			}
			break;

		case JumpState::Ascending:
			sprite.setTexture(jump_texture);
			sprite.setTextureRect(sf::IntRect({2 * FRAME_SIZE, 0}, {FRAME_SIZE, FRAME_SIZE}));
			if (velocity.y >= -PEAK_THRESHOLD)
				jumpState = JumpState::Peak;
			break;

		case JumpState::Peak:
			sprite.setTexture(jump_texture);
			sprite.setTextureRect(sf::IntRect({3 * FRAME_SIZE, 0}, {FRAME_SIZE, FRAME_SIZE}));
			if (velocity.y > PEAK_THRESHOLD)
				jumpState = JumpState::Descending;
			break;

		case JumpState::Descending:
			sprite.setTexture(jump_texture);
			sprite.setTextureRect(sf::IntRect({4 * FRAME_SIZE, 0}, {FRAME_SIZE, FRAME_SIZE}));
			break;

		case JumpState::Landing: {
			int landingFrame = 5 + currentFrame;
			sprite.setTexture(jump_texture);
			sprite.setTextureRect(sf::IntRect({landingFrame * FRAME_SIZE, 0}, {FRAME_SIZE, FRAME_SIZE}));
			frameTimer += deltaTime;
			if (frameTimer >= LAND_FRAME_DURATION) {
				frameTimer -= LAND_FRAME_DURATION;
				currentFrame++;
				if (currentFrame >= 4) {
					jumpState = JumpState::None;
					currentFrame = 0;
					frameTimer = 0.f;
				}
			}
			break;
		}
		}

		sprite.setScale({direction == Direction::Left ? -1.f : 1.f, 1.f});
	}
};
