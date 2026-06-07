#pragma once
#include "audio_service.h"

// AudioService for unit tests.
// Adjusted for testing so that tests can run without
// opening an audio device or loading files.
class NullAudioService final : public AudioService {
  public:
	NullAudioService() = default;
	~NullAudioService() override = default;
	NullAudioService(const NullAudioService &) = delete;
	NullAudioService &operator=(const NullAudioService &) = delete;
	NullAudioService(NullAudioService &&) = delete;
	NullAudioService &operator=(NullAudioService &&) = delete;

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
	float soundVolume_ = 100.f;
	float musicVolume_ = 100.f;
};
