#pragma once
#include "../../core/audio_manager.h"
#include "../../core/scene_stack.h"
#include "../dialogue_scene.h"
#include "../story_snippets.h"
#include "../victory_scene.h"
#include "main_menu.h"
#include <SFML/Graphics.hpp>
#include <memory>

// Pushes the VictoryScene that plays after the Segfault boss death animation finishes.
// On dismiss, plays the wake-up epilogue story, then returns to the main menu.
inline std::unique_ptr<Scene> makeVictoryMenu(SceneStack &stack, sf::RenderWindow &window)
{
	AudioManager::getInstance().stopAllSounds();
	AudioManager::getInstance().playMusic(MusicTrack::TRANSISTOR_BOSS_VICTORY);

	return std::make_unique<VictoryScene>(window.getSize(), [&stack, &window]() {
		stack.replace([&stack, &window]() -> std::unique_ptr<Scene> {
			return std::make_unique<DialogueScene>(
			    stack, window.getSize(), StorySnippets::epilogue(),
			    [&stack, &window]() { stack.replace([&stack, &window]() { return makeMainMenu(stack, window); }); },
			    /*opaque=*/true);
		});
	});
}
