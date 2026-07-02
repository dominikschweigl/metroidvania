#include "health_bar.h"
#include "../core/asset_manager.h"

HealthBar::HealthBar(const float indicatorGap, const float indicatorVerticalOffset, const float indicatorSize)
    : indicatorGap_(indicatorGap), indicatorVerticalOffset_(indicatorVerticalOffset), indicatorSize_(indicatorSize),
      zeroSprite_(AssetManager::getInstance().getTexture(HEALTH_ZERO_BIT)),
      oneSprite_(AssetManager::getInstance().getTexture(HEALTH_ONE_BIT))
{
	const float zeroScale = indicatorSize_ / AssetManager::getInstance().getTexture(HEALTH_ZERO_BIT).getSize().x;
	const float oneScale = indicatorSize_ / AssetManager::getInstance().getTexture(HEALTH_ONE_BIT).getSize().x;

	zeroSprite_.setScale({zeroScale, zeroScale});
	oneSprite_.setScale({oneScale, oneScale});
}

void HealthBar::draw(sf::RenderWindow &window, const Health &health, const float hotbarLeftX, const float hotbarTopY,
                     const bool diskEquipped)
{
	const sf::View previousView = window.getView();
	window.setView(sf::View(sf::FloatRect({0.f, 0.f}, sf::Vector2f(window.getSize()))));

	const float pipTopY = hotbarTopY - indicatorVerticalOffset_ - indicatorSize_;
	for (int index = 0; index < health.max; ++index) {
		const float x = hotbarLeftX + static_cast<float>(index) * (indicatorSize_ + indicatorGap_);
		sf::Sprite &sprite = (index < health.current) ? oneSprite_ : zeroSprite_;
		sprite.setPosition({x, pipTopY});
		window.draw(sprite);
	}

	if (diskEquipped)
		drawBackupDiskBadge(window, hotbarLeftX, hotbarTopY, health.max);

	window.setView(previousView);
}

void HealthBar::drawBackupDiskBadge(sf::RenderWindow &window, const float hotbarLeftX, const float hotbarTopY,
                                    const int pipCount) const
{
	const float pipTopY = hotbarTopY - indicatorVerticalOffset_ - indicatorSize_;
	const float badgeX = hotbarLeftX + static_cast<float>(pipCount) * (indicatorSize_ + indicatorGap_);
	const float badgeIconSize = indicatorSize_ - 8.f;

	const sf::Texture &tex = AssetManager::getInstance().getTexture(ITEM_BACKUP_DISK);
	sf::Sprite sprite(tex);
	const sf::Vector2u texSize = tex.getSize();
	sprite.setScale({badgeIconSize / static_cast<float>(texSize.x), badgeIconSize / static_cast<float>(texSize.y)});
	sprite.setPosition(
	    {badgeX + (indicatorSize_ - badgeIconSize) / 2.f, pipTopY + (indicatorSize_ - badgeIconSize) / 2.f});
	window.draw(sprite);
}
