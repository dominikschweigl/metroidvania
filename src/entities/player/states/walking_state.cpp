#include "walking_state.h"
#include "../../../core/audio_manager.h"
#include "../player.h"
#include <array>

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

	stepTimer += dt;
	if (stepTimer >= STEP_INTERVAL) {
		stepTimer -= STEP_INTERVAL;
		triggerStep();
	}
	return this;
}

void WalkingState::triggerStep()
{
	static constexpr std::array<SoundEffect, 3> STEP_SOUNDS = {SoundEffect::PLAYER_WALK_1, SoundEffect::PLAYER_WALK_2,
	                                                           SoundEffect::PLAYER_WALK_3};
	AudioManager::getInstance().playSound(STEP_SOUNDS[nextStepIndex], STEP_VOLUME);
	nextStepIndex = (nextStepIndex + 1) % static_cast<int>(STEP_SOUNDS.size());
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

void WalkingState::onEnter(Player & /*p*/)
{
	currentFrame = 0;
	frameTimer = 0.f;
	stepTimer = 0.f;
	nextStepIndex = 0;
	triggerStep();
}
