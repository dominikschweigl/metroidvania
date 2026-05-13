#pragma once
#include "../../core/scene_stack.h"
#include "../game_scene.h"
#include "../menu_scene.h"
#include <SFML/Graphics.hpp>
#include <memory>

inline std::unique_ptr<MenuScene> makeMainMenu(SceneStack &stack, sf::RenderWindow &window)
{
	MenuScene::Config cfg;
	cfg.title = "Segfault Slayer";
	cfg.backgroundImage = "assets/images/menu_background.png";
	cfg.buttons = {
	    {"New Game",
	     [&stack, &window]() {
		     stack.replace([&stack, &window]() -> std::unique_ptr<Scene> {
			     return std::make_unique<GameScene>(stack, window.getSize());
		     });
	     }},
	    {"Load Game", {}, false},
	    {"Settings", {}, false},
	    {"Exit", [&window]() { window.close(); }},
	};
	cfg.onEscape = [&window]() { window.close(); };
	return std::make_unique<MenuScene>(window.getSize(), std::move(cfg));
}
