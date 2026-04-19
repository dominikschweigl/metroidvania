#include "ascending_state.h"
#include "../player.h"

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

void AscendingState::applyAnimation(float dt, Player &p)
{
	p.sprite.setTexture(jump_texture);
	p.sprite.setTextureRect(sf::IntRect({2 * Player::FRAME_SIZE, 0}, {Player::FRAME_SIZE, Player::FRAME_SIZE}));
}
