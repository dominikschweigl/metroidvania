#pragma once
#include <SFML/Audio.hpp>
#include <array>
#include <cstddef>
#include <optional>
#include <string_view>
#include <unordered_map>

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

class AudioManager {
  public:
	[[nodiscard]] static AudioManager &getInstance();

	~AudioManager() = default;
	AudioManager(const AudioManager &) = delete;
	AudioManager &operator=(const AudioManager &) = delete;
	AudioManager(AudioManager &&) = delete;
	AudioManager &operator=(AudioManager &&) = delete;

	void playSound(SoundEffect sfx, float volume = 100.f);
	void stopSound(SoundEffect sfx);
	void pauseAllSounds();
	void resumeAllSounds();
	void stopAllSounds();

	void playMusic(MusicTrack track, float volume = 30.f);
	void stopMusic();
	void pauseMusic();
	void resumeMusic();
	[[nodiscard]] MusicStatus musicStatus() const;

	// Volume ranges are mapped from 0 to 100. 0 is silent, 100 is full volume.
	void setSoundVolume(float volume);
	void setMusicVolume(float volume);
	[[nodiscard]] float soundVolume() const noexcept { return soundVolume_; }
	[[nodiscard]] float musicVolume() const noexcept { return musicVolume_; }

  private:
	AudioManager() = default;

	static constexpr std::size_t VOICE_POOL_SIZE = 16;

	static std::string_view soundPath(SoundEffect sfx);
	static std::string_view musicPath(MusicTrack track);

	const sf::SoundBuffer &getBuffer(SoundEffect sfx);

	std::unordered_map<SoundEffect, sf::SoundBuffer> soundBuffers;
	std::array<std::optional<sf::Sound>, VOICE_POOL_SIZE> voices;
	std::optional<sf::Music> currentMusic;

	float soundVolume_ = 100.f;
	float musicVolume_ = 100.f;

	std::size_t bufferLoadCount = 0;
	std::size_t droppedSoundCount = 0;

	friend struct AudioManagerTestAccess;
};
