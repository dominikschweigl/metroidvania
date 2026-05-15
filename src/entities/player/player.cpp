#include "player.h"
#include "../base/entity_physics.h"

Player::Player()
    : lowerBodySprite(states.idle.idle_lower_texture),
      headSprite(AssetManager::getInstance().getTexture(PLAYER_HEAD_HAT)),
      upperBodySprite(states.idle.idle_upper_texture), currentState(&states.idle)
{
	lowerBodySprite.setOrigin({FRAME_SIZE / 2.f, static_cast<float>(FRAME_SIZE)});
	headSprite.setOrigin({FRAME_SIZE / 2.f, static_cast<float>(FRAME_SIZE)});
	upperBodySprite.setOrigin({FRAME_SIZE / 2.f, static_cast<float>(FRAME_SIZE)});
	lowerBodySprite.setPosition({15 * 32.f, 0.f});
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

	constexpr float HEAD_Y_ORIGIN_OFFSET = -(FRAME_SIZE - 4.f);
	const sf::Vector2f headPos = lowerBodySprite.getPosition() + sf::Vector2f{0.f, HEAD_Y_ORIGIN_OFFSET};

	constexpr float HAT_SPAWN_X_OFFSET = FRAME_SIZE / 2.f + 5.f;
	constexpr float HAT_SPAWN_Y_OFFSET = -FRAME_SIZE / 2.f;
	const sf::Vector2f hat_spawn_offset =
	    sf::Vector2f{static_cast<float>(direction) * HAT_SPAWN_X_OFFSET, HAT_SPAWN_Y_OFFSET};
	const sf::Vector2f spawnPos = lowerBodySprite.getPosition() + hat_spawn_offset;

	hatAbility.update(deltaTime, headPos, spawnPos, direction, velocity, world);

	updateAnimation(deltaTime);
}

void Player::updateAnimation(float dt)
{
	const float facingMultiplier = static_cast<float>(direction);
	const sf::Vector2f scale{facingMultiplier, 1.f};

	currentState->applyAnimation(dt, *this);
	lowerBodySprite.setScale(scale);
	upperBodySprite.setScale(scale);

	const sf::Vector2f upperOffset = currentState->getUpperBodyOffset();
	upperBodySprite.setPosition(lowerBodySprite.getPosition() + sf::Vector2f{upperOffset.x * scale.x, upperOffset.y});

	const bool hatAbsent = !hatAbility.isHatOnHead();
	headSprite.setTexture(AssetManager::getInstance().getTexture(hatAbsent ? PLAYER_HEAD : PLAYER_HEAD_HAT));
	headSprite.setTextureRect(sf::IntRect({0, 0}, {FRAME_SIZE, FRAME_SIZE}));
	const sf::Vector2f headOffset = currentState->getHeadOffset();
	headSprite.setPosition(lowerBodySprite.getPosition() + sf::Vector2f{headOffset.x * scale.x, headOffset.y});
	headSprite.setScale(scale);

	if (isAttackActive() && currentState->canAttack()) {
		if (meleeAttack.isMeleeActive()) {
			meleeAttack.applyAnimation(upperBodySprite, scale, upperBodySprite.getPosition());
		} else if (hatAbility.isThrowActive()) {
			hatAbility.applyAnimation(upperBodySprite, scale, upperBodySprite.getPosition());
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
	sf::Vector2f position = lowerBodySprite.getPosition();
	EntityPhysics::simulateMovement(deltaTime, position, velocity, isOnGround, GRAVITY, FRAME_SIZE, FRAME_SIZE, world);
	lowerBodySprite.setPosition(position);
	if (!old_isOnGround && isOnGround)
		transitionTo(states.landing);
}

void Player::transitionTo(PlayerState &next)
{
	currentState->onExit(*this);
	next.onEnter(*this);
	currentState = &next;
	if (!currentState->canAttack()) {
		meleeAttack.reset();
		hatAbility.reset();
	}
}

void Player::draw(sf::RenderWindow &window)
{
	if (debugHorizontalMovement)
		window.draw(debugHorizontalCollisionCheck);
	if (debugVerticalMovement)
		window.draw(debugVerticalCollisionCheck);
	window.draw(lowerBodySprite);
	window.draw(headSprite);
	window.draw(upperBodySprite);
	hatAbility.draw(window);
}
