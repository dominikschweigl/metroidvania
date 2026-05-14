#include "player.h"
#include "../base/entity_physics.h"

Player::Player()
    : sprite(states.idle.idle_texture), headSprite(AssetManager::getInstance().getTexture(PLAYER_HEAD_HAT)),
      upperSprite(meleeAttack.swing_texture), currentState(&states.idle)
{
	sprite.setOrigin({FRAME_SIZE / 2.f, static_cast<float>(FRAME_SIZE)});
	headSprite.setOrigin({FRAME_SIZE / 2.f, static_cast<float>(FRAME_SIZE)});
	upperSprite.setOrigin({FRAME_SIZE / 2.f, static_cast<float>(FRAME_SIZE)});
	sprite.setPosition({15 * 32.f, 0.f});
}

void Player::update(float deltaTime, const World &world, bool attackTriggered, bool hatThrowTriggered)
{
	handleMovement(deltaTime, world);

	PlayerState *next = currentState->update(deltaTime, *this);
	if (next != currentState)
		transitionTo(*next);

	if (attackTriggered && currentState->canAttack() && !hatAbility.isThrowActive())
		meleeAttack.trigger();

	if (hatThrowTriggered && currentState->canAttack() && hatAbility.canThrow() && !isAttackActive())
		hatAbility.trigger();

	meleeAttack.update(deltaTime);

	const sf::Vector2f headPos = sprite.getPosition() + sf::Vector2f{0.f, -(FRAME_SIZE - 4.f)};
	const sf::Vector2f spawnPos =
	    sprite.getPosition()
	    + sf::Vector2f{static_cast<float>(direction) * (FRAME_SIZE / 2.f + 5.f), -FRAME_SIZE / 2.f};
	hatAbility.update(deltaTime, headPos, spawnPos, direction, velocity, world);

	updateAnimation(deltaTime);
}

void Player::updateAnimation(float dt)
{
	sf::Vector2f scale{direction == Direction::Left ? -1.f : 1.f, 1.f};

	currentState->applyAnimation(dt, *this);
	sprite.setScale(scale);

	const bool hatAbsent = !hatAbility.isHatOnHead();
	headSprite.setTexture(AssetManager::getInstance().getTexture(hatAbsent ? PLAYER_HEAD : PLAYER_HEAD_HAT));
	headSprite.setTextureRect(sf::IntRect({0, 0}, {FRAME_SIZE, FRAME_SIZE}));
	const sf::Vector2f headOffset = currentState->getHeadOffset();
	const float mirroredX = headOffset.x * (direction == Direction::Right ? 1.f : -1.f);
	headSprite.setPosition(sprite.getPosition() + sf::Vector2f{mirroredX, headOffset.y});
	headSprite.setScale(scale);

	if (isAttackActive() && currentState->canAttack()) {
		if (meleeAttack.isMeleeActive()) {
			meleeAttack.applyAnimation(upperSprite, scale, sprite.getPosition());
		} else if (hatAbility.isThrowActive()) {
			hatAbility.applyAnimation(upperSprite, scale, sprite.getPosition());
		}
	}
}

void Player::handleMovement(float deltaTime, const World &world)
{
	inputJump = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);

	velocity.x = 0.f;
	isSprinting =
	    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift);
	float speed = isSprinting ? RUNNING_SPEED : WALKING_SPEED;

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
		velocity.x = -speed;
		direction = Direction::Left;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
		velocity.x = speed;
		direction = Direction::Right;
	}

	bool old_isOnGround = isOnGround;
	sf::Vector2f position = sprite.getPosition();
	EntityPhysics::simulateMovement(deltaTime, position, velocity, isOnGround, GRAVITY, FRAME_SIZE, FRAME_SIZE, world);
	sprite.setPosition(position);
	if (!old_isOnGround && isOnGround)
		transitionTo(states.landing);
}

void Player::draw(sf::RenderWindow &window)
{
	if (debugHorizontalMovement)
		window.draw(debugHorizontalCollisionCheck);
	if (debugVerticalMovement)
		window.draw(debugVerticalCollisionCheck);
	window.draw(sprite);
	window.draw(headSprite);
	if (isAttackActive() && currentState->canAttack())
		window.draw(upperSprite);
	hatAbility.draw(window);
}
