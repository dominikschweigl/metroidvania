#include "pre_jump_state.h"
#include "../player.h"

PlayerState *PreJumpState::update(float dt, Player &p)
{
	if (readyToAscend)
		return &p.states.ascending;
	return this;
}

void PreJumpState::applyAnimation(float dt, Player &p)
{
	p.sprite.setTexture(jump_texture);
	p.sprite.setTextureRect(
	    sf::IntRect({currentFrame * Player::FRAME_SIZE, 0}, {Player::FRAME_SIZE, Player::FRAME_SIZE}));
	frameTimer += dt;
	if (frameTimer >= PREJUMP_FRAME_DURATION) {
		frameTimer -= PREJUMP_FRAME_DURATION;
		currentFrame++;
		if (currentFrame >= 2) {
			currentFrame = 2;
			readyToAscend = true;
		}
	}
}

void PreJumpState::onEnter(Player &p)
{
	p.velocity.y = 0.f;
	currentFrame = 0;
	frameTimer = 0.f;
	readyToAscend = false;
}
