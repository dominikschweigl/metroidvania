#pragma once
#include <SFML/Graphics.hpp>

class InteractionIndicator {
  public:
	void update(float deltaTime);
	void draw(sf::RenderWindow &window, sf::FloatRect objectBounds, float playerX) const;

  private:
	[[nodiscard]] sf::Vector2f computeIndicatorCenter(sf::FloatRect objectBounds, float playerX) const;
	void drawCapsuleOutline(sf::RenderWindow &window, sf::Vector2f center, float pressDepth) const;
	void drawShadowCircle(sf::RenderWindow &window, sf::Vector2f center) const;
	void drawFaceCircle(sf::RenderWindow &window, sf::Vector2f center, float pressDepth) const;
	void drawKeyLabel(sf::RenderWindow &window, sf::Vector2f center, float pressDepth) const;

	static constexpr float RADIUS = 6.f;
	static constexpr float DEPTH_OFFSET = 1.5f;
	static constexpr float PRESS_DURATION = 0.15f;
	static constexpr float BORDER_THICKNESS = 1.2f;
	static constexpr float BIAS_FACTOR = 0.45f;
	static constexpr unsigned int LABEL_SIZE = 7;

	float pressTimer_ = 0.f;

	friend struct InteractionIndicatorTestAccess;
};
