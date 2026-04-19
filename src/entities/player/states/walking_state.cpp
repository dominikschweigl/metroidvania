#include "walking_state.h"
#include "../player.h"

PlayerState *WalkingState::update(float dt, Player &p)
{
	if (!p.isOnGround)
		return &p.states.peak;
	if (p.inputJump)
		return &p.states.preJump;
	if (p.velocity.x == 0.f)
		return &p.states.idle;
	bool sprinting =
	    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift);
	if (sprinting)
		return &p.states.running;
	return this;
}

void WalkingState::applyAnimation(float dt, Player &p)
{
	// Use extended lower-body texture when attack overlay is active
	p.sprite.setTexture(p.attackLayer.isActive() ? walk_lower_texture : walk_texture);
	frameTimer += dt;
	if (frameTimer >= WALK_FRAME_DURATION) {
		frameTimer -= WALK_FRAME_DURATION;
		currentFrame = (currentFrame + 1) % 16;
	}
	p.sprite.setTextureRect(
	    sf::IntRect({currentFrame * Player::FRAME_SIZE, 0}, {Player::FRAME_SIZE, Player::FRAME_SIZE}));
}

void WalkingState::onEnter(Player &p)
{
	currentFrame = 0;
	frameTimer = 0.f;
}
