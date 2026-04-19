#include "landing_state.h"
#include "../player.h"

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
	int landingFrame = 5 + currentFrame;
	p.sprite.setTexture(jump_texture);
	p.sprite.setTextureRect(
	    sf::IntRect({landingFrame * Player::FRAME_SIZE, 0}, {Player::FRAME_SIZE, Player::FRAME_SIZE}));
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
}

void LandingState::onEnter(Player &p)
{
	currentFrame = 0;
	frameTimer = 0.f;
	animationComplete = false;
}
