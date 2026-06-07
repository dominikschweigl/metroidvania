#pragma once

enum class SoundEffect {
	PLACEHOLDER,
	VOLUME_PREVIEW,
	PLAYER_JUMP,
	PLAYER_LAND,
	PLAYER_ATTACK_MELEE,
	PLAYER_HAT_THROW,
	PLAYER_WALK_1,
	PLAYER_WALK_2,
	PLAYER_WALK_3,
	SLIME_ATTACK,
	SLIME_JUMP,
	SLIME_MOVE,
	TRANSISTOR_BOSS_CHARGE_ATTACK_WINDUP,
	TRANSISTOR_BOSS_CHARGE_ATTACK,
	TRANSISTOR_BOSS_SHOOT_ATTACK,
	TRANSISTOR_BOSS_STEP,
	TRANSISTOR_BOSS_EXPLOSION
};

enum class MusicTrack { MAIN_MENU_THEME, GAME_THEME, GAME_OVER_THEME, AREA_1_BOSS_THEME, TRANSISTOR_BOSS_VICTORY };

enum class MusicStatus {
	Stopped,
	Playing,
	Paused,
};

// Base Class for Audio Management.
// Different specializations for game loop and testing available
class AudioService {
  public:
	virtual ~AudioService() = default;
	AudioService(const AudioService &) = delete;
	AudioService &operator=(const AudioService &) = delete;
	AudioService(AudioService &&) = delete;
	AudioService &operator=(AudioService &&) = delete;

	virtual void playSound(SoundEffect sfx, float volume = 100.f) = 0;
	virtual void stopSound(SoundEffect sfx) = 0;
	virtual void pauseAllSounds() = 0;
	virtual void resumeAllSounds() = 0;
	virtual void stopAllSounds() = 0;

	virtual void playMusic(MusicTrack track, float volume = 30.f) = 0;
	virtual void stopMusic() = 0;
	virtual void pauseMusic() = 0;
	virtual void resumeMusic() = 0;
	[[nodiscard]] virtual MusicStatus musicStatus() const = 0;

	// Volume ranges are mapped from 0 to 100. 0 is silent, 100 is full volume.
	virtual void setSoundVolume(float volume) = 0;
	virtual void setMusicVolume(float volume) = 0;
	[[nodiscard]] virtual float soundVolume() const noexcept = 0;
	[[nodiscard]] virtual float musicVolume() const noexcept = 0;

  protected:
	AudioService() = default;
};
