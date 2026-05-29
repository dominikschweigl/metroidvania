#include "descending_state.h"
#include "../player.h"

DescendingState::DescendingState()
    : jump_lower_texture(AssetManager::getInstance().getTexture(PLAYER_JUMP_LOWER_BODY)),
      jump_upper_texture(AssetManager::getInstance().getTexture(PLAYER_JUMP_UPPER_BODY))
{
}

void DescendingState::onEnter(Player &p)
{
	currentFrame = 0;
	frameTimer = 0.f;
}

PlayerState *DescendingState::update(float dt, Player &p)
{
	if (p.isAgainstLeftWall && p.inputLeft)
		return &p.states.wallSlide;
	if (p.isAgainstRightWall && p.inputRight)
		return &p.states.wallSlide;
	return this;
}

void DescendingState::applyAnimation(float dt, Player &p)
{
	const sf::IntRect frameRect({4 * Player::FRAME_SIZE, 0}, {Player::FRAME_SIZE, Player::FRAME_SIZE});
	p.lowerBodySprite.setTexture(jump_lower_texture);
	p.lowerBodySprite.setTextureRect(frameRect);
	p.upperBodySprite.setTexture(jump_upper_texture);
	p.upperBodySprite.setTextureRect(frameRect);
}
