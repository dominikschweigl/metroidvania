#include "peak_state.h"
#include "../player.h"

PeakState::PeakState() : jump_texture(AssetManager::getInstance().getTexture(PLAYER_JUMP)) {}

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

sf::Vector2f PeakState::getHeadOffset() const noexcept
{
	return HEAD_OFFSET;
}

void PeakState::applyAnimation(float dt, Player &p)
{
	p.sprite.setTexture(jump_texture);
	p.sprite.setTextureRect(sf::IntRect({3 * Player::FRAME_SIZE, 0}, {Player::FRAME_SIZE, Player::FRAME_SIZE}));
}
