#pragma once
#include <SFML/Audio.hpp>
#include <array>
#include <cstddef>
#include <optional>
#include <string_view>
#include <unordered_map>

enum class SoundEffect {
	PLACEHOLDER,
};

enum class MusicTrack {
	MAIN_MENU_THEME,
};

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

	void playSound(SoundEffect sfx);

	void playMusic(MusicTrack track);
	void stopMusic();
	void pauseMusic();
	void resumeMusic();
	[[nodiscard]] MusicStatus musicStatus() const;

  private:
	AudioManager() = default;

	static constexpr std::size_t VOICE_POOL_SIZE = 16;

	static std::string_view soundPath(SoundEffect sfx);
	static std::string_view musicPath(MusicTrack track);

	const sf::SoundBuffer &getBuffer(SoundEffect sfx);

	std::unordered_map<SoundEffect, sf::SoundBuffer> soundBuffers;
	std::array<std::optional<sf::Sound>, VOICE_POOL_SIZE> voices;
	std::optional<sf::Music> currentMusic;

	std::size_t bufferLoadCount = 0;
	std::size_t droppedSoundCount = 0;

	friend struct AudioManagerTestAccess;
};
