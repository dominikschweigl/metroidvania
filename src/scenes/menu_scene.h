#pragma once
#include "../core/scene.h"
#include "../ui/panel.h"
#include "../ui/theme.h"
#include "../ui/widget.h"
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

	using ContentFactory = std::function<std::unique_ptr<Widget>(const Theme &)>;

	// Convenience factory: builds a VerticalList of Buttons.
	[[nodiscard]] static ContentFactory buttonList(std::vector<ButtonSpec> buttons);

	struct Config {
		std::string title;
		sf::Vector2f panelSize = {420.f, 380.f};

		// When transparent, scenes beneath show through
		bool transparent = false;
		const sf::Texture *backgroundTexture = nullptr;
		sf::Color backgroundFallback = {30, 34, 60};

		bool transparentPanel = false;
		// 0-1 relative position within the background image; panel center is placed here.
		// Falls back to window-center when not set.
		std::optional<sf::Vector2f> panelImageAnchor;

		// Optional Escape handler. If empty, Escape is ignored.
		std::function<void()> onEscape;
		// When set, gates onEscape: Escape is only acted on when this returns true.
		// Blocked Escape events fall through to the panel widget.
		std::function<bool()> canEscape;

		ContentFactory contentFactory; // mandatory
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

	std::optional<Theme> theme_;

	std::optional<sf::Sprite> backgroundSprite_;
	sf::RectangleShape backgroundFallback_;

	std::unique_ptr<Panel> panel_;

	void buildPanel();
	void layoutForSize(sf::Vector2u size);
};
