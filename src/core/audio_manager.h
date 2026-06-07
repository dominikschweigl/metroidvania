#pragma once
#include "audio_service.h"
#include <SFML/Audio.hpp>
#include <array>
#include <cstddef>
#include <optional>
#include <string_view>
#include <unordered_map>

// Core Audio Service used for the game loop.
// Injected in the program entry at main.
class AudioManager : public AudioService {
  public:
	[[nodiscard]] static AudioManager &getInstance();

	~AudioManager() override = default;
	AudioManager(const AudioManager &) = delete;
	AudioManager &operator=(const AudioManager &) = delete;
	AudioManager(AudioManager &&) = delete;
	AudioManager &operator=(AudioManager &&) = delete;

	void playSound(SoundEffect sfx, float volume = 100.f) override;
	void stopSound(SoundEffect sfx) override;
	void pauseAllSounds() override;
	void resumeAllSounds() override;
	void stopAllSounds() override;

	void playMusic(MusicTrack track, float volume = 30.f) override;
	void stopMusic() override;
	void pauseMusic() override;
	void resumeMusic() override;
	[[nodiscard]] MusicStatus musicStatus() const override;

	void setSoundVolume(float volume) override;
	void setMusicVolume(float volume) override;
	[[nodiscard]] float soundVolume() const noexcept override { return soundVolume_; }
	[[nodiscard]] float musicVolume() const noexcept override { return musicVolume_; }

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
