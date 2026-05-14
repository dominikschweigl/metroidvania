#include "landing_state.h"
#include "../player.h"

LandingState::LandingState()
    : jump_lower_texture(AssetManager::getInstance().getTexture(PLAYER_JUMP_LOWER_BODY)),
      jump_upper_texture(AssetManager::getInstance().getTexture(PLAYER_JUMP_UPPER_BODY))
{
}

sf::Vector2f LandingState::getHeadOffset() const noexcept
{
	return HEAD_OFFSETS[currentFrame];
}

sf::Vector2f LandingState::getUpperBodyOffset() const noexcept
{
	return UPPER_BODY_OFFSETS[currentFrame];
}

PlayerState *LandingState::update(float dt, Player &p)
{
	if (p.inputJump)
		return &p.states.preJump;
	if (animationComplete)
		return &p.states.idle;
	return this;
}

void LandingState::applyAnimation(float dt, Player &p)
{
	frameTimer += dt;
	if (frameTimer >= LAND_FRAME_DURATION) {
		frameTimer -= LAND_FRAME_DURATION;
		currentFrame++;
		if (currentFrame >= 4) {
			currentFrame = 0;
			frameTimer = 0.f;
			animationComplete = true;
		}
	}
	const int landingFrame = 5 + currentFrame;
	const sf::IntRect frameRect({landingFrame * Player::FRAME_SIZE, 0}, {Player::FRAME_SIZE, Player::FRAME_SIZE});
	p.lowerBodySprite.setTexture(jump_lower_texture);
	p.lowerBodySprite.setTextureRect(frameRect);
	p.upperBodySprite.setTexture(jump_upper_texture);
	p.upperBodySprite.setTextureRect(frameRect);
}

void LandingState::onEnter(Player &p)
{
	currentFrame = 0;
	frameTimer = 0.f;
	animationComplete = false;
}
