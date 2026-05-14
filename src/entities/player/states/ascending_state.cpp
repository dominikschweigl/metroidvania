#include "ascending_state.h"
#include "../player.h"

AscendingState::AscendingState()
    : jump_lower_texture(AssetManager::getInstance().getTexture(PLAYER_JUMP_LOWER_BODY)),
      jump_upper_texture(AssetManager::getInstance().getTexture(PLAYER_JUMP_UPPER_BODY))
{
}

void AscendingState::onEnter(Player &p)
{
	p.velocity.y = -Player::JUMP_SPEED;
	p.isOnGround = false;
	currentFrame = 0;
	frameTimer = 0.f;
}

PlayerState *AscendingState::update(float dt, Player &p)
{
	if (p.velocity.y >= -Player::PEAK_THRESHOLD)
		return &p.states.peak;
	return this;
}

sf::Vector2f AscendingState::getHeadOffset() const noexcept
{
	return HEAD_OFFSET;
}

sf::Vector2f AscendingState::getUpperBodyOffset() const noexcept
{
	return UPPER_BODY_OFFSET;
}

void AscendingState::applyAnimation(float dt, Player &p)
{
	const sf::IntRect frameRect({2 * Player::FRAME_SIZE, 0}, {Player::FRAME_SIZE, Player::FRAME_SIZE});
	p.lowerBodySprite.setTexture(jump_lower_texture);
	p.lowerBodySprite.setTextureRect(frameRect);
	p.upperBodySprite.setTexture(jump_upper_texture);
	p.upperBodySprite.setTextureRect(frameRect);
}
