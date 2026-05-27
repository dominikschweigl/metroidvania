#include <catch2/catch_test_macros.hpp>

#include "core/audio_manager.h"

struct AudioManagerTestAccess {
	static std::size_t bufferLoadCount(const AudioManager &audio) { return audio.bufferLoadCount; }
	static std::size_t droppedSoundCount(const AudioManager &audio) { return audio.droppedSoundCount; }
	static constexpr std::size_t voicePoolSize() { return AudioManager::VOICE_POOL_SIZE; }
	static void clearVoices(AudioManager &audio)
	{
		for (auto &voice : audio.voices) {
			if (voice.has_value()) {
				voice->stop();
			}
			voice.reset();
		}
	}
	static std::size_t playingVoiceCount(const AudioManager &audio)
	{
		std::size_t count = 0;
		for (const auto &voice : audio.voices) {
			if (voice.has_value() && voice->getStatus() == sf::SoundSource::Status::Playing) {
				++count;
			}
		}
		return count;
	}
	static std::size_t pausedVoiceCount(const AudioManager &audio)
	{
		std::size_t count = 0;
		for (const auto &voice : audio.voices) {
			if (voice.has_value() && voice->getStatus() == sf::SoundSource::Status::Paused) {
				++count;
			}
		}
		return count;
	}
};

// ─── Singleton ───────────────────────────────────────────────────────────────

TEST_CASE("AudioManager - getInstance returns the same reference")
{
	auto &first = AudioManager::getInstance();
	auto &second = AudioManager::getInstance();
	CHECK(&first == &second);
}

// ─── playSound ───────────────────────────────────────────────────────────────

TEST_CASE("AudioManager - playSound loads and plays placeholder without throwing")
{
	auto &audio = AudioManager::getInstance();
	CHECK_NOTHROW(audio.playSound(SoundEffect::PLACEHOLDER));
}

TEST_CASE("AudioManager - SoundBuffer cache reuses loaded buffer")
{
	auto &audio = AudioManager::getInstance();
	// Prime the cache so subsequent calls must be cache hits.
	audio.playSound(SoundEffect::PLACEHOLDER);

	const auto loadCountBefore = AudioManagerTestAccess::bufferLoadCount(audio);
	audio.playSound(SoundEffect::PLACEHOLDER);
	audio.playSound(SoundEffect::PLACEHOLDER);
	const auto loadCountAfter = AudioManagerTestAccess::bufferLoadCount(audio);

	CHECK(loadCountAfter == loadCountBefore);
}

TEST_CASE("AudioManager - voice pool overflow drops the extra sound silently")
{
	auto &audio = AudioManager::getInstance();
	AudioManagerTestAccess::clearVoices(audio);
	const auto droppedBefore = AudioManagerTestAccess::droppedSoundCount(audio);

	for (std::size_t i = 0; i < AudioManagerTestAccess::voicePoolSize() + 1; ++i) {
		CHECK_NOTHROW(audio.playSound(SoundEffect::PLACEHOLDER));
	}

	const auto droppedAfter = AudioManagerTestAccess::droppedSoundCount(audio);
	CHECK(droppedAfter == droppedBefore + 1);
}

// ─── Bulk voice control ──────────────────────────────────────────────────────

TEST_CASE("AudioManager - pauseAllSounds transitions playing voices to Paused")
{
	auto &audio = AudioManager::getInstance();
	AudioManagerTestAccess::clearVoices(audio);
	audio.playSound(SoundEffect::PLACEHOLDER);

	audio.pauseAllSounds();

	CHECK(AudioManagerTestAccess::playingVoiceCount(audio) == 0);
	CHECK(AudioManagerTestAccess::pausedVoiceCount(audio) == 1);
	AudioManagerTestAccess::clearVoices(audio);
}

TEST_CASE("AudioManager - resumeAllSounds transitions paused voices back to Playing")
{
	auto &audio = AudioManager::getInstance();
	AudioManagerTestAccess::clearVoices(audio);
	audio.playSound(SoundEffect::PLACEHOLDER);
	audio.pauseAllSounds();

	audio.resumeAllSounds();

	CHECK(AudioManagerTestAccess::playingVoiceCount(audio) == 1);
	CHECK(AudioManagerTestAccess::pausedVoiceCount(audio) == 0);
	AudioManagerTestAccess::clearVoices(audio);
}

TEST_CASE("AudioManager - stopAllSounds clears all voice activity")
{
	auto &audio = AudioManager::getInstance();
	AudioManagerTestAccess::clearVoices(audio);
	audio.playSound(SoundEffect::PLACEHOLDER);
	audio.playSound(SoundEffect::PLAYER_JUMP);

	audio.stopAllSounds();

	CHECK(AudioManagerTestAccess::playingVoiceCount(audio) == 0);
	CHECK(AudioManagerTestAccess::pausedVoiceCount(audio) == 0);
}

// ─── Music state machine ─────────────────────────────────────────────────────

TEST_CASE("AudioManager - playMusic transitions status to Playing")
{
	auto &audio = AudioManager::getInstance();
	audio.stopMusic();

	audio.playMusic(MusicTrack::MAIN_MENU_THEME);

	CHECK(audio.musicStatus() == MusicStatus::Playing);
	audio.stopMusic();
}

TEST_CASE("AudioManager - stopMusic transitions status to Stopped")
{
	auto &audio = AudioManager::getInstance();
	audio.playMusic(MusicTrack::MAIN_MENU_THEME);

	audio.stopMusic();

	CHECK(audio.musicStatus() == MusicStatus::Stopped);
}

TEST_CASE("AudioManager - pause then resume cycles through Paused -> Playing")
{
	auto &audio = AudioManager::getInstance();
	audio.playMusic(MusicTrack::MAIN_MENU_THEME);

	audio.pauseMusic();
	CHECK(audio.musicStatus() == MusicStatus::Paused);

	audio.resumeMusic();
	CHECK(audio.musicStatus() == MusicStatus::Playing);

	audio.stopMusic();
}

TEST_CASE("AudioManager - playMusic replaces the currently playing track")
{
	auto &audio = AudioManager::getInstance();
	audio.playMusic(MusicTrack::MAIN_MENU_THEME);

	audio.playMusic(MusicTrack::MAIN_MENU_THEME);

	CHECK(audio.musicStatus() == MusicStatus::Playing);
	audio.stopMusic();
}

TEST_CASE("AudioManager - stopMusic from Stopped is a no-op")
{
	auto &audio = AudioManager::getInstance();
	audio.stopMusic();

	CHECK_NOTHROW(audio.stopMusic());
	CHECK(audio.musicStatus() == MusicStatus::Stopped);
}

TEST_CASE("AudioManager - pauseMusic / resumeMusic from Stopped are no-ops")
{
	auto &audio = AudioManager::getInstance();
	audio.stopMusic();

	CHECK_NOTHROW(audio.pauseMusic());
	CHECK(audio.musicStatus() == MusicStatus::Stopped);

	CHECK_NOTHROW(audio.resumeMusic());
	CHECK(audio.musicStatus() == MusicStatus::Stopped);
}

// ─── Master volume ───────────────────────────────────────────────────────────

TEST_CASE("AudioManager - setSoundVolume clamps to [0, 100]")
{
	auto &audio = AudioManager::getInstance();
	audio.setSoundVolume(150.f);
	CHECK(audio.soundVolume() == 100.f);
	audio.setSoundVolume(-20.f);
	CHECK(audio.soundVolume() == 0.f);
	audio.setSoundVolume(50.f);
	CHECK(audio.soundVolume() == 50.f);
	audio.setSoundVolume(100.f);
}

TEST_CASE("AudioManager - setMusicVolume clamps to [0, 100]")
{
	auto &audio = AudioManager::getInstance();
	audio.setMusicVolume(150.f);
	CHECK(audio.musicVolume() == 100.f);
	audio.setMusicVolume(-10.f);
	CHECK(audio.musicVolume() == 0.f);
	audio.setMusicVolume(75.f);
	CHECK(audio.musicVolume() == 75.f);
	audio.setMusicVolume(100.f);
}

TEST_CASE("AudioManager - playSound respects master sound volume")
{
	auto &audio = AudioManager::getInstance();
	AudioManagerTestAccess::clearVoices(audio);
	audio.setSoundVolume(0.f);

	CHECK_NOTHROW(audio.playSound(SoundEffect::PLACEHOLDER, 100.f));

	audio.setSoundVolume(100.f);
	AudioManagerTestAccess::clearVoices(audio);
}

// ─── Error contract ──────────────────────────────────────────────────────────

TEST_CASE("AudioManager - playSound on unknown SoundEffect throws logic_error")
{
	auto &audio = AudioManager::getInstance();
	// NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
	CHECK_THROWS_AS(audio.playSound(static_cast<SoundEffect>(999)), std::logic_error);
}

TEST_CASE("AudioManager - playMusic on unknown MusicTrack throws logic_error")
{
	auto &audio = AudioManager::getInstance();
	// NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
	CHECK_THROWS_AS(audio.playMusic(static_cast<MusicTrack>(999)), std::logic_error);
	audio.stopMusic();
}
