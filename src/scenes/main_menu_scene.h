#pragma once
#include "../core/scene.h"
#include "../core/scene_stack.h"
#include "../ui/panel.h"
#include "../ui/theme.h"
#include <SFML/Graphics.hpp>
#include <functional>
#include <memory>
#include <optional>

// Main menu screen. Owns its own font/theme, background sprite, and panel.
class MainMenuScene : public Scene {
  public:
	using ExitCallback = std::function<void()>;
	using NewGameFactory = std::function<std::unique_ptr<Scene>()>;

	MainMenuScene(SceneStack &stack, sf::Vector2u windowSize, NewGameFactory newGame, ExitCallback onExit);

	void handleEvent(const sf::Event &event, sf::RenderWindow &window) override;
	void update(float deltaTime) override;
	void draw(sf::RenderWindow &window) override;

  private:
	SceneStack &stack_;
	NewGameFactory newGame_;
	ExitCallback onExit_;
	sf::View uiView_;
	sf::Vector2u windowSize_;

	sf::Font font_;
	std::optional<Theme> theme_;

	sf::Texture backgroundTex_;
	sf::Sprite backgroundSprite_;
	bool hasBackground_ = false;
	sf::RectangleShape backgroundFallback_;

	std::unique_ptr<Panel> panel_;

	bool loadFont();
	void buildPanel();
	void layoutForSize(sf::Vector2u size);
};
