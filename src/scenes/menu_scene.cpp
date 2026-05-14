#include "menu_scene.h"
#include "../ui/button.h"
#include "../ui/vertical_list.h"
#include <algorithm>
#include <array>
#include <iostream>
#include <utility>

namespace {
const std::array<const char *, 7> kFontCandidates = {
    // Windows
    "C:\\Windows\\Fonts\\arial.ttf",
    "C:\\Windows\\Fonts\\segoeui.ttf",
    "C:\\Windows\\Fonts\\calibri.ttf",
    // Linux
    "/usr/share/fonts/liberation-sans-fonts/LiberationSans-Regular.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
    "/usr/share/fonts/dejavu-sans-fonts/DejaVuSans.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
};
} // namespace

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

	if (!loadFont()) {
		std::cerr << "MenuScene: no usable font found; menu will not render text.\n";
		return;
	}
	theme_.emplace(Theme{font_});
	buildPanel();
	layoutForSize(windowSize);
}

bool MenuScene::loadFont()
{
	for (const char *path : kFontCandidates) {
		if (font_.openFromFile(path))
			return true;
	}
	return false;
}

void MenuScene::buildPanel()
{
	if (!theme_)
		return;
	auto list = std::make_unique<VerticalList>(*theme_, theme_->itemSpacing);

	for (const auto &spec : config_.buttons) {
		list->addItem(std::make_unique<Button>(*theme_, spec.label, spec.onActivate, spec.enabled));
	}

	panel_ = std::make_unique<Panel>(*theme_, config_.panelSize, config_.title);
	panel_->setChild(std::move(list));
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
	if (const auto *key = event.getIf<sf::Event::KeyPressed>()) {
		if (key->code == sf::Keyboard::Key::Escape) {
			if (config_.onEscape)
				config_.onEscape();
			return;
		}
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
