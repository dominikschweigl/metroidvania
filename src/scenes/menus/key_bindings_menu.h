#pragma once
#include "../../core/scene_stack.h"
#include "../../ui/binding_list.h"
#include "../../ui/content_with_back.h"
#include "../menu_scene.h"
#include <memory>

inline std::unique_ptr<MenuScene> makeKeyBindingsMenu(SceneStack &stack, sf::Vector2u windowSize)
{
	MenuScene::Config cfg;
	cfg.title = "Controls";
	cfg.panelSize = {520.f, 600.f};
	cfg.transparent = true;

	// blPtr is shared between the factory (runs in ctor) and canEscape (called later).
	auto blPtr = std::make_shared<BindingList *>(nullptr);

	cfg.contentFactory = [&stack, blPtr, panelWidth = cfg.panelSize.x](const Theme &theme) {
		auto bl = std::make_unique<BindingList>(theme, panelWidth - 2.f * theme.itemPaddingX);
		*blPtr = bl.get();
		return std::unique_ptr<Widget>(
		    std::make_unique<ContentWithBack>(theme, std::move(bl), [&stack]() { stack.pop(); }));
	};
	cfg.canEscape = [blPtr]() { return *blPtr == nullptr || !(*blPtr)->isAwaitingInput(); };
	cfg.onEscape = [&stack]() { stack.pop(); };

	return std::make_unique<MenuScene>(windowSize, std::move(cfg));
}
