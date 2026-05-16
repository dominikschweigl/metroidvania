#pragma once
#include "../../core/scene_stack.h"
#include "../menu_scene.h"
#include "key_bindings_menu.h"
#include "main_menu.h"
#include <SFML/Graphics.hpp>
#include <memory>

inline std::unique_ptr<MenuScene> makePauseMenu(SceneStack &stack, sf::RenderWindow &window)
{
	MenuScene::Config cfg;
	cfg.title = "Segfault Slayer";
	cfg.transparent = true;
	cfg.contentFactory = MenuScene::buttonList({
	    {"Continue", [&stack]() { stack.pop(); }},
	    {"Load Game", {}, false},
	    {"Settings",
	     [&stack, &window]() {
		     stack.push([&stack, &window]() -> std::unique_ptr<Scene> {
			     return makeKeyBindingsMenu(stack, window.getSize());
		     });
	     }},
	    {"Exit to Main Menu",
	     [&stack, &window]() { stack.replace([&stack, &window]() { return makeMainMenu(stack, window); }); }},
	});
	cfg.onEscape = [&stack]() { stack.pop(); };
	return std::make_unique<MenuScene>(window.getSize(), std::move(cfg));
}
