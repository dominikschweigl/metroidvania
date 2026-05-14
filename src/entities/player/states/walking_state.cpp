#include "walking_state.h"
#include "../player.h"

WalkingState::WalkingState()
    : walk_lower_texture(AssetManager::getInstance().getTexture(PLAYER_WALK_LOWER_BODY)),
      walk_upper_texture(AssetManager::getInstance().getTexture(PLAYER_WALK_UPPER_BODY))
{
}

PlayerState *WalkingState::update(float dt, Player &p)
{
	if (!p.isOnGround)
		return &p.states.peak;
	if (p.inputJump)
		return &p.states.preJump;
	if (p.velocity.x == 0.f)
		return &p.states.idle;
	if (p.isSprinting)
		return &p.states.running;
	return this;
}

void WalkingState::applyAnimation(float dt, Player &p)
{
	frameTimer += dt;
	if (frameTimer >= WALK_FRAME_DURATION) {
		frameTimer -= WALK_FRAME_DURATION;
		currentFrame = (currentFrame + 1) % 16;
	}
	const sf::IntRect frameRect({currentFrame * Player::FRAME_SIZE, 0}, {Player::FRAME_SIZE, Player::FRAME_SIZE});
	p.lowerBodySprite.setTexture(walk_lower_texture);
	p.lowerBodySprite.setTextureRect(frameRect);
	p.upperBodySprite.setTexture(walk_upper_texture);
	p.upperBodySprite.setTextureRect(frameRect);
}

void WalkingState::onEnter(Player &p)
{
	currentFrame = 0;
	frameTimer = 0.f;
}
