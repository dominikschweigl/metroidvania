#include "health_bar.h"

namespace {
constexpr sf::Color FILLED_COLOR{200, 30, 30};
constexpr sf::Color EMPTY_COLOR{20, 20, 20};
constexpr sf::Color OUTLINE_COLOR{0, 0, 0};
constexpr float OUTLINE_THICKNESS = 3.f;
} // namespace

void HealthBar::rebuildPips(const int count)
{
	pips.clear();
	pips.reserve(count);
	for (int index = 0; index < count; ++index) {
		sf::ConvexShape pip;
		pip.setPointCount(4);
		// Diamond points relative to local origin.
		pip.setPoint(0, {0.f, -PIP_RADIUS});
		pip.setPoint(1, {PIP_RADIUS, 0.f});
		pip.setPoint(2, {0.f, PIP_RADIUS});
		pip.setPoint(3, {-PIP_RADIUS, 0.f});
		pip.setOutlineColor(OUTLINE_COLOR);
		pip.setOutlineThickness(OUTLINE_THICKNESS);

		const float centerX = MARGIN_LEFT + PIP_RADIUS + index * (2.f * PIP_RADIUS + PIP_SPACING);
		const float centerY = MARGIN_TOP + PIP_RADIUS;
		pip.setPosition({centerX, centerY});

		pips.push_back(pip);
	}
}

void HealthBar::draw(sf::RenderWindow &window, const Health &health)
{
	if (static_cast<int>(pips.size()) != health.max)
		rebuildPips(health.max);

	const sf::View previousView = window.getView();
	window.setView(window.getDefaultView());

	for (int index = 0; index < static_cast<int>(pips.size()); ++index) {
		const bool filled = index < health.current;
		pips[index].setFillColor(filled ? FILLED_COLOR : EMPTY_COLOR);
		window.draw(pips[index]);
	}

	window.setView(previousView);
}
