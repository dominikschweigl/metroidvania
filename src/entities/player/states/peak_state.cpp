#include "peak_state.h"
#include "../player.h"

PeakState::PeakState()
    : jump_lower_texture(AssetManager::getInstance().getTexture(PLAYER_JUMP_LOWER_BODY)),
      jump_upper_texture(AssetManager::getInstance().getTexture(PLAYER_JUMP_UPPER_BODY))
{
}

void PeakState::onEnter(Player &p)
{
	p.isOnGround = false;
	currentFrame = 0;
	frameTimer = 0.f;
}

PlayerState *PeakState::update(float dt, Player &p)
{
	if (p.isOnGround)
		return &p.states.landing;
	if (p.isAgainstLeftWall && p.inputLeft && p.inventory_.hasGum())
		return &p.states.wallSlide;
	if (p.isAgainstRightWall && p.inputRight && p.inventory_.hasGum())
		return &p.states.wallSlide;
	if (p.velocity.y > Player::PEAK_THRESHOLD)
		return &p.states.descending;
	return this;
}

sf::Vector2f PeakState::getHeadOffset(Player & /*p*/) const noexcept
{
	return HEAD_OFFSET;
}

sf::Vector2f PeakState::getUpperBodyOffset(Player &p) const noexcept
{
	return p.isAttackActive() ? ATTACK_UPPER_BODY_OFFSET : UPPER_BODY_OFFSET;
}

void PeakState::applyAnimation(float dt, Player &p)
{
	const sf::IntRect frameRect({3 * Player::FRAME_SIZE, 0}, {Player::FRAME_SIZE, Player::FRAME_SIZE});
	p.lowerBodySprite.setTexture(jump_lower_texture);
	p.lowerBodySprite.setTextureRect(frameRect);
	p.upperBodySprite.setTexture(jump_upper_texture);
	p.upperBodySprite.setTextureRect(frameRect);
}
