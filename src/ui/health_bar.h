#pragma once
#include "../combat/health.h"
#include <SFML/Graphics.hpp>

// Top-left HUD health indicator.
class HealthBar {
  public:
	HealthBar(const float indicatorGap, const float indicatorVerticalOffset, const float indicatorSize);
	~HealthBar() = default;
	HealthBar(const HealthBar &) = delete;
	HealthBar &operator=(const HealthBar &) = delete;
	HealthBar(HealthBar &&) = delete;
	HealthBar &operator=(HealthBar &&) = delete;

	void draw(sf::RenderWindow &window, const Health &health, float hotbarLeftX, float hotbarTopY,
	          bool diskEquipped = false);

  private:
	float indicatorGap_;
	float indicatorVerticalOffset_;
	float indicatorSize_;
	sf::Sprite zeroSprite_;
	sf::Sprite oneSprite_;

	void drawBackupDiskBadge(sf::RenderWindow &window, float hotbarLeftX, float hotbarTopY, int pipCount) const;
};
