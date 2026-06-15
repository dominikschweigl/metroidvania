#include "health_bar.h"
#include "../core/asset_manager.h"

namespace {
constexpr sf::Color FILLED_COLOR{200, 30, 30};
constexpr sf::Color EMPTY_COLOR{20, 20, 20};
constexpr sf::Color OUTLINE_COLOR{0, 0, 0};
constexpr sf::Color BADGE_FILL{20, 20, 30, 210};
constexpr sf::Color BADGE_OUTLINE{90, 90, 110};
constexpr float OUTLINE_THICKNESS = 3.f;
constexpr float BADGE_SIZE = 28.f;
constexpr float BADGE_ICON_SIZE = 22.f;
constexpr float BADGE_GAP = 10.f;
} // namespace

HealthBar::HealthBar(const float indicatorGap, const float indicatorSize)
    : indicatorGap_(indicatorGap), indicatorSize_(indicatorSize)
{}

void HealthBar::rebuildPips(const int count)
{
	pips.clear();
	pips.reserve(count);
	for (int index = 0; index < count; ++index) {
		sf::ConvexShape pip;
		pip.setPointCount(4);
		pip.setPoint(0, {0.f, -PIP_RADIUS});
		pip.setPoint(1, {PIP_RADIUS, 0.f});
		pip.setPoint(2, {0.f, PIP_RADIUS});
		pip.setPoint(3, {-PIP_RADIUS, 0.f});
		pip.setOutlineColor(OUTLINE_COLOR);
		pip.setOutlineThickness(OUTLINE_THICKNESS);
		pips.push_back(pip);
	}
}

void HealthBar::draw(sf::RenderWindow &window, const Health &health, const float hotbarLeftX, const float hotbarTopY,
                     const bool diskEquipped)
{
	if (static_cast<int>(pips.size()) != health.max)
		rebuildPips(health.max);

	const sf::View previousView = window.getView();
	window.setView(sf::View(sf::FloatRect({0.f, 0.f}, sf::Vector2f(window.getSize()))));

	const float pipCenterY = hotbarTopY - indicatorGap_ - indicatorSize_ / 2.f;
	for (int index = 0; index < static_cast<int>(pips.size()); ++index) {
		const float centerX = hotbarLeftX + PIP_RADIUS + static_cast<float>(index) * (2.f * PIP_RADIUS + PIP_SPACING);
		pips[index].setPosition({centerX, pipCenterY});
		pips[index].setFillColor(index < health.current ? FILLED_COLOR : EMPTY_COLOR);
		window.draw(pips[index]);
	}

	if (diskEquipped)
		drawBackupDiskBadge(window, hotbarLeftX, hotbarTopY, health.max);

	window.setView(previousView);
}

void HealthBar::drawBackupDiskBadge(sf::RenderWindow &window, const float hotbarLeftX, const float hotbarTopY,
                                    const int pipCount) const
{
	const float pipCenterY = hotbarTopY - indicatorGap_ - indicatorSize_ / 2.f;
	const float lastPipCenterX =
	    hotbarLeftX + PIP_RADIUS + static_cast<float>(pipCount - 1) * (2.f * PIP_RADIUS + PIP_SPACING);
	const float badgeX = lastPipCenterX + PIP_RADIUS + BADGE_GAP;
	const float badgeY = pipCenterY - BADGE_SIZE / 2.f;

	sf::RectangleShape badge({BADGE_SIZE, BADGE_SIZE});
	badge.setPosition({badgeX, badgeY});
	badge.setFillColor(BADGE_FILL);
	badge.setOutlineColor(BADGE_OUTLINE);
	badge.setOutlineThickness(1.f);
	window.draw(badge);

	const sf::Texture &tex = AssetManager::getInstance().getTexture(ITEM_BACKUP_DISK);
	sf::Sprite sprite(tex);
	const sf::Vector2u texSize = tex.getSize();
	sprite.setScale({BADGE_ICON_SIZE / static_cast<float>(texSize.x), BADGE_ICON_SIZE / static_cast<float>(texSize.y)});
	sprite.setPosition({badgeX + (BADGE_SIZE - BADGE_ICON_SIZE) / 2.f, badgeY + (BADGE_SIZE - BADGE_ICON_SIZE) / 2.f});
	window.draw(sprite);
}
