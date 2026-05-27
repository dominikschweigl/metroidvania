#pragma once
#include "../../core/scene_stack.h"
#include "../../ui/confirm_dialog.h"
#include "../menu_scene.h"
#include <SFML/Graphics.hpp>
#include <functional>
#include <memory>
#include <string>
#include <utility>

inline std::unique_ptr<MenuScene> makeConfirmationMenu(SceneStack &stack, sf::Vector2u windowSize, std::string prompt,
                                                       std::function<void()> onConfirm)
{
	MenuScene::Config cfg;
	cfg.panelSize = {520.f, 200.f};
	cfg.transparent = true;

	cfg.contentFactory = [prompt = std::move(prompt), onConfirm = std::move(onConfirm), &stack](const Theme &theme) {
		auto onYes = [&stack, onConfirm]() {
			onConfirm();
			stack.pop();
		};
		auto onNo = [&stack]() { stack.pop(); };
		return std::unique_ptr<Widget>(
		    std::make_unique<ConfirmDialog>(theme, prompt, std::move(onYes), std::move(onNo)));
	};
	cfg.onEscape = [&stack]() { stack.pop(); };

	return std::make_unique<MenuScene>(windowSize, std::move(cfg));
}
