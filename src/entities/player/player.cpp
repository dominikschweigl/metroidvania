#include "player.h"

Player::Player()
	: sprite(states.idle.idle_texture), upperSprite(attackLayer.swing_texture),
	  currentState(&states.idle) {
	sprite.setOrigin({FRAME_SIZE / 2.f, static_cast<float>(FRAME_SIZE)});
	upperSprite.setOrigin({FRAME_SIZE / 2.f, static_cast<float>(FRAME_SIZE)});
	sprite.setPosition({5 * 32.f, 0.f});
}

void Player::update(float deltaTime, const World *world, bool attackTriggered) {
	handleMovement(deltaTime, world);

	PlayerState *next = currentState->update(deltaTime, *this);
	if (next != currentState)
		transitionTo(*next);

	if (attackTriggered && currentState->canAttack())
		attackLayer.trigger();
	attackLayer.update(deltaTime);

	updateAnimation(deltaTime);
}

void Player::updateAnimation(float dt) {
	sf::Vector2f scale{direction == Direction::Left ? -1.f : 1.f, 1.f};

	currentState->applyAnimation(dt, *this);
	sprite.setScale(scale);

	if (attackLayer.isActive() && currentState->canAttack()) {
		attackLayer.applyAnimation(upperSprite, scale, sprite.getPosition());
	}
}

void Player::handleMovement(float deltaTime, const World *world) {
	inputJump = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);

	velocity.x = 0.f;
	isSprinting = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ||
				  sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift);
	float speed = isSprinting ? RUNNING_SPEED : WALKING_SPEED;

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
		velocity.x = -speed;
		direction = Direction::Left;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
		velocity.x = speed;
		direction = Direction::Right;
	}

	if (world == nullptr)
		return;

	if (!isGroundBelow(*world)) {
		isOnGround = false;
		velocity.y += GRAVITY * deltaTime;
	}

	float futureX = handleHorizontalMovement(*world, deltaTime);
	float futureY = handleVerticalMovement(*world, deltaTime);
	sprite.setPosition({futureX, futureY});
}

void Player::draw(sf::RenderWindow &window) {
	if (debugHorizontalMovement)
		window.draw(debugHorizontalCollisionCheck);
	if (debugVerticalMovement)
		window.draw(debugVerticalCollisionCheck);
	window.draw(sprite);
	if (attackLayer.isActive() && currentState->canAttack())
		window.draw(upperSprite);
}
