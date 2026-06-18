#pragma once
#include "../../core/scene_stack.h"
#include "../menu_scene.h"
#include <SFML/Graphics.hpp>
#include <memory>

// Stage 2 -> 3 interrupt: a full-screen "bluescreen" that freezes the fight until
// the player dismisses it with the single Continue button.
inline std::unique_ptr<MenuScene> makeBluescreenMenu(SceneStack &stack, sf::RenderWindow &window)
{
	MenuScene::Config cfg;
	cfg.title = "SEGMENTATION FAULT (core dumped)";
	cfg.transparent = false;
	cfg.backgroundFallback = {0, 0, 170};
	cfg.contentFactory = MenuScene::buttonList({
	    {"Continue", [&stack]() { stack.pop(); }},
	});
	cfg.onEscape = [&stack]() { stack.pop(); };
	return std::make_unique<MenuScene>(window.getSize(), std::move(cfg));
}
