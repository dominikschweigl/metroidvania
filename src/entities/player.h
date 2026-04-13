#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

#include "../world/world.h"

class Player {
  public:
	enum class Direction { Left, Right };
	Direction direction = Direction::Left;

	static constexpr float GRAVITY = 1200.f;
	static constexpr float WALKING_SPEED = 200.f;
	static constexpr float RUNNING_SPEED = 350.f;
	static constexpr float JUMP_SPEED = 500.f;

	static constexpr int FRAME_SIZE = 32;
	static constexpr float WALK_FRAME_DURATION = 1 / 10.f;
	static constexpr float PREJUMP_FRAME_DURATION = 0.08f;
	static constexpr float LAND_FRAME_DURATION = 0.07f;
	static constexpr float PEAK_THRESHOLD = 250.f; // velocity.y range considered "peak"

	const sf::Texture idle_texture{"./assets/images/player/idle_hat.png"};
	const sf::Texture idle_lower_texture{"./assets/images/player/idle_lower_body_extended.png"};
	const sf::Texture walk_texture{"./assets/images/player/walk.png"};
	const sf::Texture walk_lower_texture{"./assets/images/player/walk_lower_body_extended.png"};
	const sf::Texture walk_upper_texture{"./assets/images/player/walk_upper_body.png"};
	const sf::Texture jump_texture{"./assets/images/player/jump.png"};
	const sf::Texture run_texture{"./assets/images/player/run.png"};
	const sf::Texture run_lower_texture{"./assets/images/player/run_lower_body_extended.png"};
	const sf::Texture run_upper_texture{"./assets/images/player/run_upper_body.png"};
	const sf::Texture attack_swing_upper_texture{"./assets/images/player/attack_swing_upper_body_extended.png"};
	const sf::Texture attack_overhead_upper_texture{"./assets/images/player/attack_overhead_upper_body_extended.png"};

	sf::Sprite sprite;
	sf::Sprite upperSprite;

	bool isOnGround = true;
	sf::Vector2f velocity;

	// Each entry defines one attack in the combo chain.
	// To extend to 3 combos: push another AttackDef into comboChain in the constructor.
	struct AttackDef {
		const sf::Texture *upperTexture;
		int frameCount;
		float frameDuration;
	};
	std::vector<AttackDef> comboChain;

	Player() : sprite(idle_texture), upperSprite(walk_upper_texture)
	{
		sprite.setOrigin({FRAME_SIZE / 2.f, static_cast<float>(FRAME_SIZE)});
		upperSprite.setOrigin({FRAME_SIZE / 2.f, static_cast<float>(FRAME_SIZE)});
		comboChain = {
		    {&attack_swing_upper_texture, 8, 0.09f},
		    {&attack_overhead_upper_texture, 8, 0.09f},
		};
	}
	~Player() = default;

	sf::FloatRect getBounds() const {
    return sf::Rect<float>(
        {sprite.getPosition().x - FRAME_SIZE / 2.f, sprite.getPosition().y - FRAME_SIZE},
        {FRAME_SIZE, FRAME_SIZE}
    );
}

	void update(float deltaTime, const World *world = nullptr, bool attackTriggered = false)
	{
		handleMovement(deltaTime, world);
		updateAnimation(deltaTime, attackTriggered);
	}

	void draw(sf::RenderWindow &window)
	{
		window.draw(sprite);
		if (drawUpperSprite)
			window.draw(upperSprite);
	}

	sf::Vector2f getPosition() const { return sprite.getPosition(); }

  private:
	enum class JumpState { None, PreJump, Ascending, Peak, Descending, Landing };
	JumpState jumpState = JumpState::None;

	int currentFrame = 0;
	float frameTimer = 0.f;

	// Attack state — independent from jump/walk counters
	int attackComboIndex = -1; // -1 = not attacking
	bool comboQueued = false;
	int attackFrame = 0;
	float attackFrameTimer = 0.f;

	bool drawUpperSprite = false;

	void handleMovement(float deltaTime, const World* world = nullptr)
	{
		velocity.x = 0.f;
		bool isSprinting = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)
		                   || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift);
		float speed = isSprinting ? RUNNING_SPEED : WALKING_SPEED;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
			velocity.x = -speed;
			direction = Direction::Left;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
			velocity.x = speed;
			direction = Direction::Right;
		}

		bool jumpPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);

		if (isOnGround && jumpPressed && jumpState == JumpState::None) {
			jumpState = JumpState::PreJump;
			currentFrame = 0;
			frameTimer = 0.f;
		}

		// Delay takeoff until pre-jump animation finishes
		if (jumpState == JumpState::PreJump) {
			velocity.y = 0.f;
		} else if (jumpState == JumpState::Ascending && isOnGround) {
			velocity.y = -JUMP_SPEED;
			isOnGround = false;
		}
		if (!isOnGround) {
			velocity.y += GRAVITY * deltaTime;
		}

		if (world) {
			sf::FloatRect bounds = getBounds();
			if (world->willCollideWithWall(bounds, velocity, deltaTime, *world)) {
				velocity.x = 0.f;
			}
		}

		sprite.move(velocity * deltaTime);

		if (world) {
			sf::FloatRect bounds = getBounds();
			auto groundY = world->getGroundYAt(bounds);

			if (groundY.has_value() && velocity.y > 0.f) {
				float playerBottom = sprite.getPosition().y;
				if (playerBottom >= groundY.value()) {
					sprite.setPosition({sprite.getPosition().x, groundY.value()});
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
			} else if (!groundY.has_value() && isOnGround) {
				isOnGround = false; // Player walked off platform
			}
		} else {
			if (sprite.getPosition().y >= 0.f && velocity.y > 0.f) {
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

	void updateAnimation(float deltaTime, bool attackTriggered)
	{
		if (attackTriggered) {
			if (attackComboIndex == -1) {
				// Start first attack in chain
				attackComboIndex = 0;
				attackFrame = 0;
				attackFrameTimer = 0.f;
				comboQueued = false;
			} else if (attackComboIndex < static_cast<int>(comboChain.size()) - 1) {
				// Queue next attack in chain
				comboQueued = true;
			}
		}

		// --- Advance attack animation ---
		if (attackComboIndex >= 0) {
			attackFrameTimer += deltaTime;
			const AttackDef &attack = comboChain[attackComboIndex];
			if (attackFrameTimer >= attack.frameDuration) {
				attackFrameTimer -= attack.frameDuration;
				attackFrame++;
				if (attackFrame >= attack.frameCount) {
					if (comboQueued && attackComboIndex < static_cast<int>(comboChain.size()) - 1) {
						comboQueued = false;
						attackComboIndex++;
						attackFrame = 0;
						attackFrameTimer = 0.f;
					} else {
						attackComboIndex = -1;
						comboQueued = false;
					}
				}
			}
		}

		bool isWalking = velocity.x != 0.f;
		bool isSprinting = isWalking
		                   && (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)
		                       || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift));
		bool isJumping = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);

		// Interrupt landing with jump input
		if (jumpState == JumpState::Landing && isJumping) {
			jumpState = JumpState::PreJump;
			currentFrame = 0;
		}

		// --- Walking/running while attacking: layered sprites ---
		if (attackComboIndex >= 0 && isWalking && jumpState == JumpState::None) {
			const AttackDef &attack = comboChain[attackComboIndex];
			sf::Vector2f scale{direction == Direction::Left ? -1.f : 1.f, 1.f};

			// Lower body: walk or run legs
			if (isSprinting) {
				sprite.setTexture(run_lower_texture);
				frameTimer += deltaTime;
				if (frameTimer >= WALK_FRAME_DURATION) {
					frameTimer -= static_cast<int>(frameTimer / WALK_FRAME_DURATION) * WALK_FRAME_DURATION;
					currentFrame = (currentFrame + 1) % 8;
				}
			} else {
				sprite.setTexture(walk_lower_texture);
				frameTimer += deltaTime;
				if (frameTimer >= WALK_FRAME_DURATION) {
					frameTimer -= WALK_FRAME_DURATION;
					currentFrame = (currentFrame + 1) % 16;
				}
			}
			sprite.setTextureRect(sf::IntRect({currentFrame * FRAME_SIZE, 0}, {FRAME_SIZE, FRAME_SIZE}));
			sprite.setScale(scale);

			// Upper body: attack upper-body sprite
			upperSprite.setTexture(*attack.upperTexture);
			upperSprite.setTextureRect(sf::IntRect({attackFrame * FRAME_SIZE, 0}, {FRAME_SIZE, FRAME_SIZE}));
			upperSprite.setPosition(sprite.getPosition());
			upperSprite.setScale(scale);
			drawUpperSprite = true;
			return;
		}

		// --- Attack while standing: idle lower body + attack upper body ---
		if (attackComboIndex >= 0) {
			const AttackDef &attack = comboChain[attackComboIndex];
			sf::Vector2f scale{direction == Direction::Left ? -1.f : 1.f, 1.f};

			sprite.setTexture(idle_lower_texture);
			sprite.setTextureRect(sf::IntRect({0, 0}, {FRAME_SIZE, FRAME_SIZE}));
			sprite.setScale(scale);

			upperSprite.setTexture(*attack.upperTexture);
			upperSprite.setTextureRect(sf::IntRect({attackFrame * FRAME_SIZE, 0}, {FRAME_SIZE, FRAME_SIZE}));
			upperSprite.setPosition(sprite.getPosition());
			upperSprite.setScale(scale);
			drawUpperSprite = true;
			return;
		}

		drawUpperSprite = false;

		// --- Normal movement animation ---
		switch (jumpState) {
		case JumpState::None:
			if (isSprinting) {
				sprite.setTexture(run_texture);
				frameTimer += deltaTime;
				if (frameTimer >= WALK_FRAME_DURATION) {
					frameTimer -= static_cast<int>(frameTimer / WALK_FRAME_DURATION) * WALK_FRAME_DURATION;
					currentFrame = (currentFrame + 1) % 8;
				}
				sprite.setTextureRect(sf::IntRect({currentFrame * FRAME_SIZE, 0}, {FRAME_SIZE, FRAME_SIZE}));
			} else if (isWalking) {
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
