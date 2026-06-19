#pragma once
#include "../core/scene.h"
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>

// Full-screen terminal-style victory screen shown after the Segfault boss is defeated.
// Locks out input for a few seconds then calls onContinue on any key press.
class VictoryScene : public Scene {
  public:
	VictoryScene(sf::Vector2u windowSize, std::function<void()> onContinue);

	~VictoryScene() override = default;
	VictoryScene(const VictoryScene &) = delete;
	VictoryScene &operator=(const VictoryScene &) = delete;
	VictoryScene(VictoryScene &&) = delete;
	VictoryScene &operator=(VictoryScene &&) = delete;

	void handleEvent(const sf::Event &event, sf::RenderWindow &window) override;
	void update(float deltaTime) override;
	void draw(sf::RenderWindow &window) override;

  private:
	sf::Vector2u windowSize_;
	sf::View uiView_;
	const sf::Font &font_;
	std::function<void()> onContinue_;
	float elapsedSeconds_ = 0.f;

	[[nodiscard]] bool canContinue() const noexcept;
	void layoutForSize(sf::Vector2u size);
	void drawScanlines(sf::RenderWindow &window) const;
	void drawTextLine(sf::RenderWindow &window, const std::string &str, unsigned int charSize, sf::Color color,
	                  float x, float &curY, bool centered = false) const;
};
