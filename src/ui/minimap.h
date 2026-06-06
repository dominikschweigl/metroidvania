#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

// Top-left HUD health indicator.
class MiniMap {
  public:
	static constexpr float PIP_RADIUS = 22.f;
	static constexpr float PIP_SPACING = 12.f;
	static constexpr float MARGIN_LEFT = 28.f;
	static constexpr float MARGIN_TOP = 28.f;

	MiniMap() = default;
	~MiniMap() = default;
	MiniMap(const MiniMap &) = delete;
	MiniMap &operator=(const MiniMap &) = delete;
	MiniMap(MiniMap &&) = delete;
	MiniMap &operator=(MiniMap &&) = delete;

	// Draws the minimap in screen-space using the window's default view.
	void draw(sf::RenderWindow &window, const sf::Vector2f playerPos);
};
