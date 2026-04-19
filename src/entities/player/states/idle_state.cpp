#include "idle_state.h"
#include "../player.h"

PlayerState *IdleState::update(float dt, Player &p)
{
	if (!p.isOnGround)
		return &p.states.peak;
	if (p.inputJump)
		return &p.states.preJump;
	if (p.velocity.x != 0.f) {
		return p.isSprinting ? static_cast<PlayerState *>(&p.states.running)
		                     : static_cast<PlayerState *>(&p.states.walking);
	}
	return this;
}

void IdleState::applyAnimation(float dt, Player &p)
{
	// Use extended lower-body texture when attack overlay is active
	p.sprite.setTexture(p.attackLayer.isActive() ? idle_lower_texture : idle_texture);
	p.sprite.setTextureRect(sf::IntRect({0, 0}, {Player::FRAME_SIZE, Player::FRAME_SIZE}));
}

void IdleState::onEnter(Player &p)
{
	currentFrame = 0;
	frameTimer = 0.f;
}
