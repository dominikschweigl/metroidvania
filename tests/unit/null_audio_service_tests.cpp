#include <catch2/catch_test_macros.hpp>

#include "core/audio_service.h"
#include "core/null_audio_service.h"

// NullAudioService lets audio-triggering
// code run in tests without an audio device or asset files.

TEST_CASE("NullAudioService - audio calls are silent no-ops through the interface")
{
	NullAudioService null;
	AudioService &audio = null;

	CHECK_NOTHROW(audio.playSound(SoundEffect::PLAYER_LAND, 20.f));
	CHECK_NOTHROW(audio.playMusic(MusicTrack::MAIN_MENU_THEME));
	CHECK_NOTHROW(audio.stopAllSounds());
	CHECK(audio.musicStatus() == MusicStatus::Stopped);
}

TEST_CASE("NullAudioService - volume getters echo the last set value, clamped")
{
	NullAudioService null;
	AudioService &audio = null;

	audio.setSoundVolume(150.f);
	CHECK(audio.soundVolume() == 100.f);

	audio.setMusicVolume(-10.f);
	CHECK(audio.musicVolume() == 0.f);

	audio.setMusicVolume(60.f);
	CHECK(audio.musicVolume() == 60.f);
}
