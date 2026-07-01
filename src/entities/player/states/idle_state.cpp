#include "idle_state.h"
#include "../player.h"

IdleState::IdleState()
    : idle_lower_texture(AssetManager::getInstance().getTexture(PLAYER_IDLE_LOWER_BODY)),
      idle_upper_texture(AssetManager::getInstance().getTexture(PLAYER_IDLE_UPPER_BODY))
{
}

PlayerState *IdleState::update(float /*dt*/, Player &p)
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

void IdleState::applyAnimation(float /*dt*/, Player &p)
{
	p.lowerBodySprite.setTexture(idle_lower_texture);
	p.lowerBodySprite.setTextureRect(sf::IntRect({0, 0}, {Player::FRAME_SIZE, Player::FRAME_SIZE}));
	p.upperBodySprite.setTexture(idle_upper_texture);
	p.upperBodySprite.setTextureRect(sf::IntRect({0, 0}, {Player::FRAME_SIZE, Player::FRAME_SIZE}));
}

void IdleState::onEnter(Player & /*p*/)
{
	currentFrame = 0;
	frameTimer = 0.f;
}
