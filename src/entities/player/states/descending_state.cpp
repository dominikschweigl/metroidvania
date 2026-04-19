#include "descending_state.h"
#include "../player.h"

void DescendingState::onEnter(Player &p)
{
	currentFrame = 0;
	frameTimer = 0.f;
}

PlayerState *DescendingState::update(float dt, Player &p)
{
	return this;
}

void DescendingState::applyAnimation(float dt, Player &p)
{
	p.sprite.setTexture(jump_texture);
	p.sprite.setTextureRect(sf::IntRect({4 * Player::FRAME_SIZE, 0}, {Player::FRAME_SIZE, Player::FRAME_SIZE}));
}
