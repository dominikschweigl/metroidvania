#pragma once
#include "../../core/audio_manager.h"
#include "../../core/input_manager.h"
#include "../../core/scene_stack.h"
#include "../game_scene.h"
#include "../menu_scene.h"
#include "main_menu.h"
#include <SFML/Graphics.hpp>
#include <memory>

inline std::unique_ptr<MenuScene> makeGameOverMenu(SceneStack &stack, sf::RenderWindow &window)
{
	AudioManager::getInstance().stopAllSounds();
	AudioManager::getInstance().playMusic(MusicTrack::GAME_OVER_THEME);

	MenuScene::Config cfg;
	cfg.title = "Game Over";
	cfg.transparent = true;
	cfg.contentFactory = MenuScene::buttonList({
	    {"Try Again",
	     [&stack, &window]() {
		     InputManager::getInstance().suppressPlayerActions();
		     stack.replace([&stack, &window]() -> std::unique_ptr<Scene> {
			     return std::make_unique<GameScene>(stack, window, worldName, true);
		     });
	     }},
	    {"Exit to Main Menu",
	     [&stack, &window]() { stack.replace([&stack, &window]() { return makeMainMenu(stack, window); }); }},
	});
	cfg.onEscape = [&stack, &window]() { stack.replace([&stack, &window]() { return makeMainMenu(stack, window); }); };
	return std::make_unique<MenuScene>(window.getSize(), std::move(cfg));
}
