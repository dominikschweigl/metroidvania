#include "audio_manager.h"
#include <algorithm>
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

void AudioManager::playSound(const SoundEffect sfx, const float volume)
{
	const sf::SoundBuffer &buffer = getBuffer(sfx);
	const float effective = std::clamp(volume * soundVolume_ / 100.f, 0.f, 100.f);

	for (auto &voice : voices) {
		if (!voice.has_value()) {
			voice.emplace(buffer);
			voice->setVolume(effective);
			voice->play();
			return;
		}
		if (voice->getStatus() == sf::SoundSource::Status::Stopped) {
			voice->setBuffer(buffer);
			voice->setVolume(effective);
			voice->play();
			return;
		}
	}

	++droppedSoundCount;
}

void AudioManager::stopSound(const SoundEffect sfx)
{
	const auto iterator = soundBuffers.find(sfx);
	if (iterator == soundBuffers.end())
		return;

	const sf::SoundBuffer *const buffer = &iterator->second;
	for (auto &voice : voices) {
		if (voice.has_value() && &voice->getBuffer() == buffer)
			voice->stop();
	}
}

void AudioManager::pauseAllSounds()
{
	for (auto &voice : voices) {
		if (voice.has_value() && voice->getStatus() == sf::SoundSource::Status::Playing) {
			voice->pause();
		}
	}
}

void AudioManager::resumeAllSounds()
{
	for (auto &voice : voices) {
		if (voice.has_value() && voice->getStatus() == sf::SoundSource::Status::Paused) {
			voice->play();
		}
	}
}

void AudioManager::stopAllSounds()
{
	for (auto &voice : voices) {
		if (voice.has_value()) {
			voice->stop();
		}
	}
}

void AudioManager::playMusic(const MusicTrack track, const float volume)
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
	const float effective = std::clamp(volume * musicVolume_ / 100.f, 0.f, 100.f);
	currentMusic->setVolume(effective);
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

void AudioManager::setSoundVolume(const float volume)
{
	soundVolume_ = std::clamp(volume, 0.f, 100.f);
}

void AudioManager::setMusicVolume(const float volume)
{
	musicVolume_ = std::clamp(volume, 0.f, 100.f);
	if (currentMusic.has_value()) {
		currentMusic->setVolume(musicVolume_);
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
	case SoundEffect::VOLUME_PREVIEW:
		return "./assets/audio/placeholder.ogg";
	case SoundEffect::PLAYER_JUMP:
		return "./assets/audio/player/player_jump.wav";
	case SoundEffect::PLAYER_LAND:
		return "./assets/audio/player/player_land.wav";
	case SoundEffect::PLAYER_ATTACK_MELEE:
		return "./assets/audio/player/player_attack_melee.wav";
	case SoundEffect::PLAYER_HAT_THROW:
		return "./assets/audio/player/player_hat_throw.wav";
	case SoundEffect::PLAYER_WALK_1:
		return "./assets/audio/player/player_walk_1.wav";
	case SoundEffect::PLAYER_WALK_2:
		return "./assets/audio/player/player_walk_2.wav";
	case SoundEffect::PLAYER_WALK_3:
		return "./assets/audio/player/player_walk_3.wav";
	case SoundEffect::SLIME_ATTACK:
		return "./assets/audio/entities/race_condition_slime/race_condition_slime_attack.wav";
	case SoundEffect::SLIME_JUMP:
		return "./assets/audio/entities/race_condition_slime/race_condition_slime_jump.wav";
	case SoundEffect::SLIME_MOVE:
		return "./assets/audio/entities/race_condition_slime/race_condition_slime_move.wav";
	case SoundEffect::TRANSISTOR_BOSS_CHARGE_ATTACK_WINDUP:
		return "./assets/audio/entities/bosses/transistor_boss/transistor_boss_charge_attack_windup.wav";
	case SoundEffect::TRANSISTOR_BOSS_CHARGE_ATTACK:
		return "./assets/audio/entities/bosses/transistor_boss/transistor_boss_charge_attack.wav";
	case SoundEffect::TRANSISTOR_BOSS_SHOOT_ATTACK:
		return "./assets/audio/entities/bosses/transistor_boss/transistor_boss_shoot_attack.wav";
	case SoundEffect::TRANSISTOR_BOSS_STEP:
		return "./assets/audio/entities/bosses/transistor_boss/transistor_boss_step.wav";
	}
	throw std::logic_error("soundPath: missing SoundEffect path entry");
}

std::string_view AudioManager::musicPath(const MusicTrack track)
{
	switch (track) {
	case MusicTrack::MAIN_MENU_THEME:
		return "./assets/audio/music/main_menu_theme.ogg";
	case MusicTrack::GAME_THEME:
		return "./assets/audio/music/game_theme.ogg";
	case MusicTrack::GAME_OVER_THEME:
		return "./assets/audio/music/game_over_theme.ogg";
	case MusicTrack::AREA_1_BOSS_THEME:
		return "./assets/audio/music/area_1_boss_theme.ogg";
	}
	throw std::logic_error("musicPath: missing MusicTrack path entry");
}
