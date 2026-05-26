#include "running_state.h"
#include "../../../core/audio_manager.h"
#include "../player.h"

RunningState::RunningState()
    : run_lower_texture(AssetManager::getInstance().getTexture(PLAYER_RUN_LOWER_BODY)),
      run_upper_texture(AssetManager::getInstance().getTexture(PLAYER_RUN_UPPER_BODY))
{
}

sf::Vector2f RunningState::getHeadOffset() const noexcept
{
	return HEAD_OFFSETS[currentFrame];
}

PlayerState *RunningState::update(float dt, Player &p)
{
	if (!p.isOnGround)
		return &p.states.peak;
	if (p.inputJump)
		return &p.states.preJump;
	if (p.velocity.x == 0.f)
		return &p.states.idle;
	if (!p.isSprinting)
		return &p.states.walking;

	stepTimer += dt;
	if (stepTimer >= STEP_INTERVAL) {
		stepTimer -= STEP_INTERVAL;
		triggerStep();
	}
	return this;
}

void RunningState::triggerStep()
{
	static constexpr std::array<SoundEffect, 3> STEP_SOUNDS = {SoundEffect::PLAYER_WALK_1, SoundEffect::PLAYER_WALK_2,
	                                                           SoundEffect::PLAYER_WALK_3};
	AudioManager::getInstance().playSound(STEP_SOUNDS[nextStepIndex], STEP_VOLUME);
	nextStepIndex = (nextStepIndex + 1) % static_cast<int>(STEP_SOUNDS.size());
}

void RunningState::applyAnimation(float dt, Player &p)
{
	frameTimer += dt;
	if (frameTimer >= WALK_FRAME_DURATION) {
		frameTimer -= static_cast<int>(frameTimer / WALK_FRAME_DURATION) * WALK_FRAME_DURATION;
		currentFrame = (currentFrame + 1) % 8;
	}
	const sf::IntRect frameRect({currentFrame * Player::FRAME_SIZE, 0}, {Player::FRAME_SIZE, Player::FRAME_SIZE});
	p.lowerBodySprite.setTexture(run_lower_texture);
	p.lowerBodySprite.setTextureRect(frameRect);
	p.upperBodySprite.setTexture(run_upper_texture);
	p.upperBodySprite.setTextureRect(frameRect);
}

void RunningState::onEnter(Player & /*p*/)
{
	currentFrame = 0;
	frameTimer = 0.f;
	stepTimer = 0.f;
	nextStepIndex = 0;
	triggerStep();
}
