#include "null_audio_service.h"
#include <algorithm>

void NullAudioService::playSound(SoundEffect, float) {}
void NullAudioService::stopSound(SoundEffect) {}
void NullAudioService::pauseAllSounds() {}
void NullAudioService::resumeAllSounds() {}
void NullAudioService::stopAllSounds() {}

void NullAudioService::playMusic(MusicTrack, float) {}
void NullAudioService::stopMusic() {}
void NullAudioService::pauseMusic() {}
void NullAudioService::resumeMusic() {}

MusicStatus NullAudioService::musicStatus() const
{
	return MusicStatus::Stopped;
}

void NullAudioService::setSoundVolume(const float volume)
{
	soundVolume_ = std::clamp(volume, 0.f, 100.f);
}

void NullAudioService::setMusicVolume(const float volume)
{
	musicVolume_ = std::clamp(volume, 0.f, 100.f);
}
