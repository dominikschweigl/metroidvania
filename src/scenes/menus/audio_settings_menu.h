#pragma once
#include "../../core/scene_stack.h"
#include "../../ui/audio_settings_list.h"
#include "../../ui/content_with_back.h"
#include "../menu_scene.h"
#include <memory>

inline std::unique_ptr<MenuScene> makeAudioSettingsMenu(SceneStack &stack, sf::Vector2u windowSize)
{
	MenuScene::Config cfg;
	cfg.title = "Audio";
	cfg.panelSize = {520.f, 380.f};
	cfg.transparent = true;

	cfg.contentFactory = [&stack, panelWidth = cfg.panelSize.x](const Theme &theme) {
		const float contentWidth = panelWidth - 2.f * theme.itemPaddingX;
		auto list = std::make_unique<AudioSettingsList>(theme, contentWidth);
		return std::unique_ptr<Widget>(
		    std::make_unique<ContentWithBack>(theme, std::move(list), [&stack]() { stack.pop(); }));
	};
	cfg.onEscape = [&stack]() { stack.pop(); };

	return std::make_unique<MenuScene>(windowSize, std::move(cfg));
}
