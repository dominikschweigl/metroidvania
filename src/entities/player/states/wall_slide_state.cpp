#include "wall_slide_state.h"
#include "../player.h"

WallSlideState::WallSlideState()
    : wall_slide_lower_texture(AssetManager::getInstance().getTexture(PLAYER_WALL_SLIDE_LOWER_BODY)),
      wall_slide_upper_texture(AssetManager::getInstance().getTexture(PLAYER_WALL_SLIDE_UPPER_BODY))
{
}

void WallSlideState::onEnter(Player &p)
{
	p.isOnGround = false;
	originalGravity = p.gravity;
	p.gravity *= WALL_SLIDE_GRAVITY_FACTOR;
	p.velocity.y *= 0.1f;
	wallDirection = p.isAgainstLeftWall ? Direction::Left : Direction::Right;
	currentFrame = 0;
	frameTimer = 0.f;
}

void WallSlideState::onExit(Player &p)
{
	p.gravity = originalGravity;
}

PlayerState *WallSlideState::update(float /*dt*/, Player &p)
{
	if (p.isOnGround) {
		return &p.states.landing;
	}
	if (p.inputJump) {
		p.wallJumpTimer = Player::WALL_JUMP_DURATION;
		p.velocity.x = wallDirection == Direction::Left ? Player::RUNNING_SPEED : -Player::RUNNING_SPEED;
		p.setDirection(wallDirection == Direction::Left ? Direction::Right : Direction::Left);
		return &p.states.ascending;
	}

	const bool stillAgainstWall = wallDirection == Direction::Left ? p.isAgainstLeftWall : p.isAgainstRightWall;
	const bool pressingIntoWall = wallDirection == Direction::Left ? p.inputLeft : p.inputRight;
	if (!stillAgainstWall || !pressingIntoWall)
		return &p.states.descending;

	return this;
}

sf::Vector2f WallSlideState::getHeadOffset(Player & /*p*/) const noexcept
{
	return HEAD_OFFSET;
}

sf::Vector2f WallSlideState::getUpperBodyOffset(Player & /*p*/) const noexcept
{
	return UPPER_BODY_OFFSET;
}

void WallSlideState::applyAnimation(float /*dt*/, Player &p)
{
	const sf::IntRect frameRect({0, 0}, {Player::FRAME_SIZE, Player::FRAME_SIZE});
	p.lowerBodySprite.setTexture(wall_slide_lower_texture);
	p.lowerBodySprite.setTextureRect(frameRect);
	p.upperBodySprite.setTexture(wall_slide_upper_texture);
	p.upperBodySprite.setTextureRect(frameRect);
}
