#pragma once
#include "../../core/scene_stack.h"
#include "../menu_scene.h"
#include "audio_settings_menu.h"
#include "key_bindings_menu.h"
#include <SFML/Graphics.hpp>
#include <memory>

inline std::unique_ptr<MenuScene> makeSettingsMenu(SceneStack &stack, sf::Vector2u windowSize)
{
	MenuScene::Config cfg;
	cfg.title = "Settings";
	cfg.panelSize = {420.f, 380.f};
	cfg.transparent = true;
	cfg.contentFactory = MenuScene::buttonList({
	    {"Audio",
	     [&stack, windowSize]() {
		     stack.push(
		         [&stack, windowSize]() -> std::unique_ptr<Scene> { return makeAudioSettingsMenu(stack, windowSize); });
	     }},
	    {"Controls",
	     [&stack, windowSize]() {
		     stack.push(
		         [&stack, windowSize]() -> std::unique_ptr<Scene> { return makeKeyBindingsMenu(stack, windowSize); });
	     }},
	    {"Back", [&stack]() { stack.pop(); }},
	});
	cfg.onEscape = [&stack]() { stack.pop(); };
	return std::make_unique<MenuScene>(windowSize, std::move(cfg));
}
