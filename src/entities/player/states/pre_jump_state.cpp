#include "pre_jump_state.h"
#include "../../../core/audio_manager.h"
#include "../player.h"

PreJumpState::PreJumpState()
    : jump_lower_texture(AssetManager::getInstance().getTexture(PLAYER_JUMP_LOWER_BODY)),
      jump_upper_texture(AssetManager::getInstance().getTexture(PLAYER_JUMP_UPPER_BODY))
{
}

sf::Vector2f PreJumpState::getHeadOffset(Player & /*p*/) const noexcept
{
	return HEAD_OFFSETS[currentFrame];
}

sf::Vector2f PreJumpState::getUpperBodyOffset(Player &p) const noexcept
{
	return p.isAttackActive() ? ATTACK_UPPER_BODY_OFFSETS[currentFrame] : UPPER_BODY_OFFSETS[currentFrame];
}

PlayerState *PreJumpState::update(float dt, Player &p)
{
	if (readyToAscend)
		return &p.states.ascending;
	return this;
}

void PreJumpState::applyAnimation(float dt, Player &p)
{
	frameTimer += dt;
	if (frameTimer >= PREJUMP_FRAME_DURATION) {
		frameTimer -= PREJUMP_FRAME_DURATION;
		currentFrame++;
		if (currentFrame >= 2) {
			currentFrame = 1;
			readyToAscend = true;
		}
	}
	const sf::IntRect frameRect({currentFrame * Player::FRAME_SIZE, 0}, {Player::FRAME_SIZE, Player::FRAME_SIZE});
	p.lowerBodySprite.setTexture(jump_lower_texture);
	p.lowerBodySprite.setTextureRect(frameRect);
	p.upperBodySprite.setTexture(jump_upper_texture);
	p.upperBodySprite.setTextureRect(frameRect);
}

void PreJumpState::onEnter(Player &p)
{
	AudioManager::getInstance().playSound(SoundEffect::PLAYER_JUMP, 20.f);
	p.velocity.y = 0.f;
	currentFrame = 0;
	frameTimer = 0.f;
	readyToAscend = false;
}
