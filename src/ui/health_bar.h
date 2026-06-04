#pragma once
#include "../combat/health.h"
#include <SFML/Graphics.hpp>
#include <vector>

// Top-left HUD health indicator.
class HealthBar {
  public:
	static constexpr float PIP_RADIUS = 22.f;
	static constexpr float PIP_SPACING = 12.f;
	static constexpr float MARGIN_LEFT = 28.f;
	static constexpr float MARGIN_TOP = 28.f;

	HealthBar() = default;
	~HealthBar() = default;
	HealthBar(const HealthBar &) = delete;
	HealthBar &operator=(const HealthBar &) = delete;
	HealthBar(HealthBar &&) = delete;
	HealthBar &operator=(HealthBar &&) = delete;

	// Draws the health bar in screen-space using the window's default view.
	// Pass diskEquipped=true to show the Backup Disk badge to the right of the pips.
	void draw(sf::RenderWindow &window, const Health &health, bool diskEquipped = false);

  private:
	void rebuildPips(int count);
	void drawBackupDiskBadge(sf::RenderWindow &window, int pipCount) const;

	std::vector<sf::ConvexShape> pips;
};
