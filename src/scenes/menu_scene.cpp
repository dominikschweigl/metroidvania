#include "menu_scene.h"
#include "../core/asset_manager.h"
#include "../core/input_manager.h"
#include "../ui/button.h"
#include "../ui/vertical_list.h"
#include <algorithm>
#include <utility>

MenuScene::MenuScene(sf::Vector2u windowSize, Config config) : config_(std::move(config)), windowSize_(windowSize)
{
	uiView_.setSize({static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)});
	uiView_.setCenter(uiView_.getSize() / 2.f);

	if (!config_.transparent) {
		if (config_.backgroundTexture) {
			backgroundSprite_.emplace(*config_.backgroundTexture);
		} else {
			backgroundFallback_.setFillColor(config_.backgroundFallback);
		}
	}

	theme_.emplace(Theme{AssetManager::getInstance().getFont(UI_FONT)});
	buildPanel();
	layoutForSize(windowSize);
}

MenuScene::ContentFactory MenuScene::buttonList(std::vector<ButtonSpec> buttons)
{
	return [buttons = std::move(buttons)](const Theme &theme) {
		auto list = std::make_unique<VerticalList>(theme, theme.itemSpacing);
		for (const auto &spec : buttons)
			list->addItem(std::make_unique<Button>(theme, spec.label, spec.onActivate, spec.enabled));
		return std::unique_ptr<Widget>(std::move(list));
	};
}

void MenuScene::buildPanel()
{
	if (!theme_ || !config_.contentFactory)
		return;
	panel_ = std::make_unique<Panel>(*theme_, config_.panelSize, config_.title);
	panel_->setChild(config_.contentFactory(*theme_));
}

void MenuScene::layoutForSize(sf::Vector2u size)
{
	windowSize_ = size;
	uiView_.setSize({static_cast<float>(size.x), static_cast<float>(size.y)});
	uiView_.setCenter(uiView_.getSize() / 2.f);

	if (backgroundSprite_) {
		const auto tex = config_.backgroundTexture->getSize();
		if (tex.x > 0 && tex.y > 0) {
			constexpr float fill = 0.85f;
			const float scale = std::min(static_cast<float>(size.x) / tex.x, static_cast<float>(size.y) / tex.y) * fill;
			backgroundSprite_->setScale({scale, scale});
			const float w = tex.x * scale;
			const float h = tex.y * scale;
			backgroundSprite_->setPosition({(size.x - w) * 0.5f, (size.y - h) * 0.5f});
		}
	} else if (!config_.transparent) {
		backgroundFallback_.setSize({static_cast<float>(size.x), static_cast<float>(size.y)});
	}

	if (panel_) {
		const auto ps = panel_->getSize();
		panel_->setPosition({(size.x - ps.x) * 0.5f, (size.y - ps.y) * 0.5f});
	}
}

void MenuScene::handleEvent(const sf::Event &event, sf::RenderWindow &window)
{
	if (const auto *resized = event.getIf<sf::Event::Resized>()) {
		layoutForSize({resized->size.x, resized->size.y});
		window.setView(uiView_);
		return;
	}
	// Bind the UI view so widget hit-testing via window.getView() is consistent.
	window.setView(uiView_);
	InputManager &input = InputManager::getInstance();
	const bool blocked = config_.canEscape && !config_.canEscape();
	if (!blocked && config_.onEscape && input.consume(MenuAction::Back)) {
		config_.onEscape();
		return;
	}
	if (panel_)
		panel_->handleEvent(event, window);
}

void MenuScene::update(float deltaTime)
{
	if (panel_)
		panel_->update(deltaTime);
}

void MenuScene::draw(sf::RenderWindow &window)
{
	window.setView(uiView_);
	if (!config_.transparent) {
		window.clear();
		if (backgroundSprite_)
			window.draw(*backgroundSprite_);
		else
			window.draw(backgroundFallback_);
	}
	if (panel_)
		panel_->draw(window);
}
