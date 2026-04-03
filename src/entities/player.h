#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class Player {
  public:
	enum class Direction { Left, Right };
	Direction direction = Direction::Left;

	static constexpr float GRAVITY = 1000.f;   // pixels per second squared
	static constexpr float SPEED = 200.f;      // pixels per second
	static constexpr float JUMP_SPEED = 500.f; // initial jump velocity

	const std::string TEXTURE_PATH = "./assets/images/player/idle.png";
	const sf::Texture texture;
	sf::Sprite sprite;

	bool isOnGround = true;
	sf::Vector2f velocity;

	Player() : texture(TEXTURE_PATH), sprite(texture)
	{
		sprite.setOrigin({texture.getSize().x / 2.f, static_cast<float>(texture.getSize().y)});
	}
	~Player() = default;

	void update(float deltaTime)
	{
		handleMovement(deltaTime);
		updateAnimation();
	}

	sf::Vector2f getPosition() const { return sprite.getPosition(); }

  private:
	void handleMovement(float deltaTime)
	{
		velocity.x = 0.f; // Reset horizontal velocity each frame

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
			velocity.x = -SPEED;
			direction = Direction::Left;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
			velocity.x = SPEED;
			direction = Direction::Right;
		}

		if (isOnGround && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
			velocity.y = -JUMP_SPEED;
			isOnGround = false;
		}
		if (!isOnGround) {
			velocity.y += GRAVITY * deltaTime; // Apply gravity
		}

		sprite.move(velocity * deltaTime);

		if (sprite.getPosition().y >= 0.f) {
			sprite.setPosition({sprite.getPosition().x, 0.f});
			velocity.y = 0.f;
			isOnGround = true;
		}
	};

	void updateAnimation()
	{
		if (direction == Direction::Left) {
			sprite.setScale({1.f, 1.f});
		} else {
			sprite.setScale({-1.f, 1.f});
		}
	}
};
