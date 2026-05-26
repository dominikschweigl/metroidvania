#pragma once
#include "../../core/input_manager.h"
#include "../../core/scene_stack.h"
#include "../../ui/binding_list.h"
#include "../../ui/content_with_back.h"
#include "../menu_scene.h"
#include "confirmation_menu.h"
#include <memory>
#include <string>

inline std::unique_ptr<MenuScene> makeKeyBindingsMenu(SceneStack &stack, sf::Vector2u windowSize)
{
	MenuScene::Config cfg;
	cfg.title = "Controls";
	cfg.panelSize = {520.f, 600.f};
	cfg.transparent = true;

	// blPtr is shared between the factory (runs in ctor) and canEscape (called later).
	auto blPtr = std::make_shared<BindingList *>(nullptr);

	auto rebindHandler = [&stack, blPtr, windowSize](GameAction action, InputBinding binding) {
		auto &input = InputManager::getInstance();
		const auto conflict = input.findConflict(binding, action);
		if (!conflict.has_value()) {
			input.rebind(action, binding);
			if (*blPtr)
				(*blPtr)->refresh();
			return;
		}

		std::string conflictName = "another action";
		for (const auto &meta : InputManager::gameActions()) {
			if (meta.action == *conflict) {
				conflictName = std::string(meta.displayName);
				break;
			}
		}
		std::string prompt = "Reassign from \"" + conflictName + "\"?";

		auto onConfirm = [blPtr, action, binding]() {
			InputManager::getInstance().rebind(action, binding);
			if (*blPtr)
				(*blPtr)->refresh();
		};
		stack.push([&stack, windowSize, prompt = std::move(prompt), onConfirm = std::move(onConfirm)]() {
			return makeConfirmationMenu(stack, windowSize, prompt, onConfirm);
		});
	};

	cfg.contentFactory = [&stack, blPtr, panelWidth = cfg.panelSize.x,
	                      rebindHandler = std::move(rebindHandler)](const Theme &theme) {
		auto bl = std::make_unique<BindingList>(theme, panelWidth - 2.f * theme.itemPaddingX, rebindHandler);
		*blPtr = bl.get();
		return std::unique_ptr<Widget>(
		    std::make_unique<ContentWithBack>(theme, std::move(bl), [&stack]() { stack.pop(); }));
	};
	cfg.canEscape = [blPtr]() { return *blPtr == nullptr || !(*blPtr)->isAwaitingInput(); };
	cfg.onEscape = [&stack]() { stack.pop(); };

	return std::make_unique<MenuScene>(windowSize, std::move(cfg));
}
