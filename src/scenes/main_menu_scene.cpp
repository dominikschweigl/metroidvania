#include "../ui/button.h"
#include "../ui/vertical_list.h"
#include "main_menu_scene.h"
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

MainMenuScene::MainMenuScene(SceneStack &stack, sf::Vector2u windowSize, NewGameFactory newGame, ExitCallback onExit)
    : stack_(stack), newGame_(std::move(newGame)), onExit_(std::move(onExit)), windowSize_(windowSize),
      backgroundSprite_(backgroundTex_)
{
	uiView_.setSize({static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)});
	uiView_.setCenter(uiView_.getSize() / 2.f);

	if (backgroundTex_.loadFromFile("assets/images/menu_background.png")) {
		hasBackground_ = true;
		backgroundSprite_.setTexture(backgroundTex_, true);
	} else {
		backgroundFallback_.setFillColor({30, 34, 60});
	}

	if (!loadFont()) {
		std::cerr << "MainMenuScene: no usable font found; menu will not render text.\n";
		return;
	}
	theme_.emplace(Theme{font_});
	buildPanel();
	layoutForSize(windowSize);
}

bool MainMenuScene::loadFont()
{
	for (const char *path : kFontCandidates) {
		if (font_.openFromFile(path))
			return true;
	}
	return false;
}

void MainMenuScene::buildPanel()
{
	if (!theme_)
		return;
	auto list = std::make_unique<VerticalList>(*theme_, theme_->itemSpacing);

	list->addItem(std::make_unique<Button>(*theme_, "New Game", [this]() {
		if (!newGame_)
			return;
		// Capture by value so the factory survives this scene being replaced.
		auto factory = newGame_;
		stack_.replace([factory = std::move(factory)]() { return factory(); });
	}));
	list->addItem(std::make_unique<Button>(*theme_, "Load Game", []() {}, /*enabled=*/false));
	list->addItem(std::make_unique<Button>(*theme_, "Settings", []() {}, /*enabled=*/false));
	list->addItem(std::make_unique<Button>(*theme_, "Exit", [this]() {
		if (onExit_)
			onExit_();
	}));

	panel_ = std::make_unique<Panel>(*theme_, sf::Vector2f{420.f, 380.f}, "Metroidvania");
	panel_->setChild(std::move(list));
}

void MainMenuScene::layoutForSize(sf::Vector2u size)
{
	windowSize_ = size;
	uiView_.setSize({static_cast<float>(size.x), static_cast<float>(size.y)});
	uiView_.setCenter(uiView_.getSize() / 2.f);

	if (hasBackground_) {
		const auto tex = backgroundTex_.getSize();
		if (tex.x > 0 && tex.y > 0) {
			backgroundSprite_.setScale({static_cast<float>(size.x) / tex.x, static_cast<float>(size.y) / tex.y});
		}
	} else {
		backgroundFallback_.setSize({static_cast<float>(size.x), static_cast<float>(size.y)});
	}

	if (panel_) {
		const auto ps = panel_->getSize();
		panel_->setPosition({(size.x - ps.x) * 0.5f, (size.y - ps.y) * 0.5f});
	}
}

void MainMenuScene::handleEvent(const sf::Event &event, sf::RenderWindow &window)
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
			if (onExit_)
				onExit_();
			return;
		}
	}
	if (panel_)
		panel_->handleEvent(event, window);
}

void MainMenuScene::update(float deltaTime)
{
	if (panel_)
		panel_->update(deltaTime);
}

void MainMenuScene::draw(sf::RenderWindow &window)
{
	window.setView(uiView_);
	window.clear();
	if (hasBackground_)
		window.draw(backgroundSprite_);
	else
		window.draw(backgroundFallback_);
	if (panel_)
		panel_->draw(window);
}
