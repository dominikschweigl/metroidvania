#pragma once
#include "../../core/audio_manager.h"
#include "../../core/input_manager.h"
#include "../../core/scene_stack.h"
#include "../game_scene.h"
#include "../menu_scene.h"
#include "settings_menu.h"
#include <SFML/Graphics.hpp>
#include <memory>

inline std::unique_ptr<MenuScene> makeMainMenu(SceneStack &stack, sf::RenderWindow &window)
{
	AudioManager::getInstance().stopAllSounds();
	AudioManager::getInstance().playMusic(MusicTrack::MAIN_MENU_THEME);

	MenuScene::Config cfg;
	cfg.title = "Segfault Slayer";
	cfg.backgroundTexture = &AssetManager::getInstance().getTexture(MAIN_MENU_BACKGROUND);
	cfg.contentFactory = MenuScene::buttonList({
	    {"New Game",
	     [&stack, &window]() {
		     InputManager::getInstance().suppressPlayerActions();
		     stack.replace([&stack, &window]() -> std::unique_ptr<Scene> {
			     return std::make_unique<GameScene>(stack, window, worldName, true);
		     });
	     }},
	    {"Load Game",
	     [&stack, &window]() {
		     InputManager::getInstance().suppressPlayerActions();
		     stack.replace([&stack, &window]() -> std::unique_ptr<Scene> {
			     return std::make_unique<GameScene>(stack, window, worldName, false);
		     });
	     }},
	    {"Settings",
	     [&stack, &window]() {
		     stack.push(
		         [&stack, &window]() -> std::unique_ptr<Scene> { return makeSettingsMenu(stack, window.getSize()); });
	     }},
	    {"Exit", [&window]() { window.close(); }},
	});
	cfg.onEscape = [&window]() { window.close(); };
	return std::make_unique<MenuScene>(window.getSize(), std::move(cfg));
}
