#include "death_state.h"
#include "../../../../../core/audio_manager.h"
#include "../transistor_boss.h"

namespace transistor_boss {

EnemyState *DeathState::update(float /*deltaTime*/, BaseEnemy & /*enemy*/, const World & /*world*/,
                               sf::Vector2f /*playerPos*/)
{
	// Boss never leaves this state.
	return this;
}

void DeathState::updateAnimation(float deltaTime, BaseEnemy &enemy)
{
	auto &transistor_boss = static_cast<TransistorBoss &>(enemy);

	frameTimer += deltaTime;

	constexpr int WINDUP_FRAME_COUNT = 16;
	constexpr int RECOVER_FRAME_COUNT = 17;
	constexpr float STAGE_FRAME_DURATION = 0.1f;

	switch (phase) {
	case Phase::WindupFirst:
	case Phase::WindupSecond: {
		if (currentFrame < WINDUP_FRAME_COUNT - 1) {
			if (frameTimer >= STAGE_FRAME_DURATION) {
				frameTimer -= STAGE_FRAME_DURATION;
				++currentFrame;
			}
			transistor_boss.setAnimation(TransistorBoss::TransistorBossAnimation::ChargeAttackWindup, currentFrame);
			return;
		}

		AudioManager::getInstance().stopSound(SoundEffect::TRANSISTOR_BOSS_CHARGE_ATTACK_WINDUP);
		currentFrame = 0;
		frameTimer = 0.f;

		if (phase == Phase::WindupFirst) {
			phase = Phase::Recover;
			transistor_boss.setAnimation(TransistorBoss::TransistorBossAnimation::Recover, currentFrame);
		} else {
			phase = Phase::Explosion;
			AudioManager::getInstance().playSound(SoundEffect::TRANSISTOR_BOSS_EXPLOSION);
			transistor_boss.setAnimation(TransistorBoss::TransistorBossAnimation::Death, currentFrame);
		}
		return;
	}

	case Phase::Recover: {
		if (currentFrame < RECOVER_FRAME_COUNT - 1) {
			if (frameTimer >= STAGE_FRAME_DURATION) {
				frameTimer -= STAGE_FRAME_DURATION;
				++currentFrame;
			}
			transistor_boss.setAnimation(TransistorBoss::TransistorBossAnimation::Recover, currentFrame);
			return;
		}

		phase = Phase::WindupSecond;
		currentFrame = 0;
		frameTimer = 0.f;
		AudioManager::getInstance().playSound(SoundEffect::TRANSISTOR_BOSS_CHARGE_ATTACK_WINDUP);
		transistor_boss.setAnimation(TransistorBoss::TransistorBossAnimation::ChargeAttackWindup, currentFrame);
		return;
	}

	case Phase::Explosion: {
		constexpr int FRAME_COUNT = 9;
		constexpr float FRAME_DURATION = 0.12f;
		constexpr int LAST_FRAME = FRAME_COUNT - 1;

		if (currentFrame < LAST_FRAME) {
			if (frameTimer >= FRAME_DURATION) {
				frameTimer -= FRAME_DURATION;
				++currentFrame;
			}
		} else if (!victoryMusicStarted) {
			AudioManager::getInstance().playMusic(MusicTrack::TRANSISTOR_BOSS_VICTORY);
			victoryMusicStarted = true;
		}

		transistor_boss.setAnimation(TransistorBoss::TransistorBossAnimation::Death, currentFrame);
		return;
	}
	}
}

void DeathState::onEnter(BaseEnemy &enemy)
{
	auto &transistor_boss = static_cast<TransistorBoss &>(enemy);
	transistor_boss.setVelocityX(0.f);
	transistor_boss.setAuraPhase(TransistorBoss::AuraPhase::None);
	transistor_boss.setInvincible(true);
	phase = Phase::WindupFirst;
	currentFrame = 0;
	frameTimer = 0.f;
	victoryMusicStarted = false;

	AudioManager::getInstance().stopMusic();
	AudioManager::getInstance().playSound(SoundEffect::TRANSISTOR_BOSS_CHARGE_ATTACK_WINDUP);
}

json DeathState::serialize() const
{
	json j = EnemyState::serialize();

	j["type"] = "DeathState";

	return j;
}

} // namespace transistor_boss
