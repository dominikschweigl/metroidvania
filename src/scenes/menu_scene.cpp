#include "menu_scene.h"
#include "../ui/button.h"
#include "../ui/vertical_list.h"
#include <array>
#include <iostream>
#include <utility>

namespace {
const std::array<const char *, 5> kFontCandidates = {
    "assets/fonts/menu.ttf",
    "/usr/share/fonts/liberation-sans-fonts/LiberationSans-Regular.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
    "/usr/share/fonts/dejavu-sans-fonts/DejaVuSans.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
};
} // namespace

MenuScene::MenuScene(sf::Vector2u windowSize, Config config)
    : config_(std::move(config)), windowSize_(windowSize), backgroundSprite_(backgroundTex_)
{
	uiView_.setSize({static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)});
	uiView_.setCenter(uiView_.getSize() / 2.f);

	if (!config_.transparent) {
		if (config_.backgroundImage && backgroundTex_.loadFromFile(*config_.backgroundImage)) {
			hasBackground_ = true;
			backgroundSprite_.setTexture(backgroundTex_, true);
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

	if (hasBackground_) {
		const auto tex = backgroundTex_.getSize();
		if (tex.x > 0 && tex.y > 0) {
			backgroundSprite_.setScale({static_cast<float>(size.x) / tex.x, static_cast<float>(size.y) / tex.y});
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
		if (hasBackground_)
			window.draw(backgroundSprite_);
		else
			window.draw(backgroundFallback_);
	}
	if (panel_)
		panel_->draw(window);
}
