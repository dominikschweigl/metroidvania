#include "peak_state.h"
#include "../player.h"

void PeakState::onEnter(Player &p)
{
	currentFrame = 0;
	frameTimer = 0.f;
}

PlayerState *PeakState::update(float dt, Player &p)
{
	if (p.velocity.y > Player::PEAK_THRESHOLD)
		return &p.states.descending;
	return this;
}

void PeakState::applyAnimation(float dt, Player &p)
{
	p.sprite.setTexture(jump_texture);
	p.sprite.setTextureRect(sf::IntRect({3 * Player::FRAME_SIZE, 0}, {Player::FRAME_SIZE, Player::FRAME_SIZE}));
}
