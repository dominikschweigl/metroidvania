#pragma once
#include "../../core/audio_manager.h"
#include "../../core/scene_stack.h"
#include "../victory_scene.h"
#include "main_menu.h"
#include <SFML/Graphics.hpp>
#include <memory>

// Pushes the VictoryScene that plays after the Segfault boss death animation finishes.
// On dismiss, replaces the full scene stack with the main menu.
inline std::unique_ptr<Scene> makeVictoryMenu(SceneStack &stack, sf::RenderWindow &window)
{
	AudioManager::getInstance().stopAllSounds();
	AudioManager::getInstance().playMusic(MusicTrack::TRANSISTOR_BOSS_VICTORY);

	return std::make_unique<VictoryScene>(window.getSize(), [&stack, &window]() {
		stack.replace([&stack, &window]() { return makeMainMenu(stack, window); });
	});
}
