#pragma once
#include "../core/scene.h"
#include "../ui/panel.h"
#include "../ui/theme.h"
#include <SFML/Graphics.hpp>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

// Generic menu screen.
class MenuScene : public Scene {
  public:
	struct ButtonSpec {
		std::string label;
		std::function<void()> onActivate;
		bool enabled = true;
	};

	struct Config {
		std::string title;
		std::vector<ButtonSpec> buttons;
		sf::Vector2f panelSize = {420.f, 380.f};

		// When transparent, scenes beneath show through
		bool transparent = false;
		const sf::Texture *backgroundTexture = nullptr;
		sf::Color backgroundFallback = {30, 34, 60};

		// Optional Escape handler. If empty, Escape is ignored.
		std::function<void()> onEscape;
	};

	MenuScene(sf::Vector2u windowSize, Config config);

	void handleEvent(const sf::Event &event, sf::RenderWindow &window) override;
	void update(float deltaTime) override;
	void draw(sf::RenderWindow &window) override;
	bool isTransparent() const override { return config_.transparent; }

  private:
	Config config_;
	sf::View uiView_;
	sf::Vector2u windowSize_;

	sf::Font font_;
	std::optional<Theme> theme_;

	std::optional<sf::Sprite> backgroundSprite_;
	sf::RectangleShape backgroundFallback_;

	std::unique_ptr<Panel> panel_;

	bool loadFont();
	void buildPanel();
	void layoutForSize(sf::Vector2u size);
};
