#pragma once
#include "../combat/health.h"
#include <SFML/Graphics.hpp>
#include <vector>

// Top-left HUD health indicator.
class HealthBar {
  public:
	static constexpr float PIP_RADIUS = 13.f;
	static constexpr float PIP_SPACING = 6.f;

	HealthBar(float indicatorGap, float indicatorSize);
	~HealthBar() = default;
	HealthBar(const HealthBar &) = delete;
	HealthBar &operator=(const HealthBar &) = delete;
	HealthBar(HealthBar &&) = delete;
	HealthBar &operator=(HealthBar &&) = delete;

	void draw(sf::RenderWindow &window, const Health &health, float hotbarLeftX, float hotbarTopY,
	          bool diskEquipped = false);

  private:
	float indicatorGap_;
	float indicatorSize_;

	void rebuildPips(int count);
	void drawBackupDiskBadge(sf::RenderWindow &window, float hotbarLeftX, float hotbarTopY, int pipCount) const;

	std::vector<sf::ConvexShape> pips;
};
