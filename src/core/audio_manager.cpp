#include "audio_manager.h"
#include <format>
#include <stdexcept>
#include <string>

AudioManager &AudioManager::getInstance()
{
	static AudioManager instance;
	return instance;
}

const sf::SoundBuffer &AudioManager::getBuffer(const SoundEffect sfx)
{
	const auto path = soundPath(sfx);
	const auto [iterator, isInserted] = soundBuffers.try_emplace(sfx);
	if (isInserted) {
		if (!iterator->second.loadFromFile(std::string(path))) {
			soundBuffers.erase(iterator);
			throw std::runtime_error(std::format("AudioManager: failed to load sound: {}", path));
		}
		++bufferLoadCount;
	}
	return iterator->second;
}

void AudioManager::playSound(const SoundEffect sfx)
{
	const sf::SoundBuffer &buffer = getBuffer(sfx);

	for (auto &voice : voices) {
		if (!voice.has_value()) {
			voice.emplace(buffer);
			voice->play();
			return;
		}
		if (voice->getStatus() == sf::SoundSource::Status::Stopped) {
			voice->setBuffer(buffer);
			voice->play();
			return;
		}
	}

	++droppedSoundCount;
}

void AudioManager::playMusic(const MusicTrack track)
{
	const auto path = musicPath(track);

	if (!currentMusic.has_value()) {
		currentMusic.emplace();
	} else {
		currentMusic->stop();
	}
	if (!currentMusic->openFromFile(std::string(path))) {
		throw std::runtime_error(std::format("AudioManager: failed to load music: {}", path));
	}
	currentMusic->setLooping(true);
	currentMusic->play();
}

void AudioManager::stopMusic()
{
	if (currentMusic.has_value()) {
		currentMusic->stop();
	}
}

void AudioManager::pauseMusic()
{
	if (currentMusic.has_value() && currentMusic->getStatus() == sf::SoundSource::Status::Playing) {
		currentMusic->pause();
	}
}

void AudioManager::resumeMusic()
{
	if (currentMusic.has_value() && currentMusic->getStatus() == sf::SoundSource::Status::Paused) {
		currentMusic->play();
	}
}

MusicStatus AudioManager::musicStatus() const
{
	if (!currentMusic.has_value()) {
		return MusicStatus::Stopped;
	}
	switch (currentMusic->getStatus()) {
	case sf::SoundSource::Status::Playing:
		return MusicStatus::Playing;
	case sf::SoundSource::Status::Paused:
		return MusicStatus::Paused;
	case sf::SoundSource::Status::Stopped:
		return MusicStatus::Stopped;
	}
	return MusicStatus::Stopped;
}

std::string_view AudioManager::soundPath(const SoundEffect sfx)
{
	switch (sfx) {
	case SoundEffect::PLACEHOLDER:
		return "./assets/audio/placeholder.ogg";
	}
	throw std::logic_error("soundPath: missing SoundEffect path entry");
}

std::string_view AudioManager::musicPath(const MusicTrack track)
{
	switch (track) {
	case MusicTrack::MAIN_MENU_THEME:
		return "./assets/audio/main_menu.ogg";
	}
	throw std::logic_error("musicPath: missing MusicTrack path entry");
}
