#include "minimap.h"
#include "../core/asset_manager.h"

void MiniMap::draw(sf::RenderWindow &window, const sf::Vector2f playerPos)
{
	const sf::View previousView = window.getView();
	window.setView(sf::View(sf::FloatRect({0.f, 0.f}, sf::Vector2f(window.getSize()))));

	const sf::Texture &minimap = AssetManager::getInstance().getTexture(TextureAsset::MINIMAP);
	const sf::Vector2u texSize = minimap.getSize();
	sf::Sprite sprite(minimap);
	sprite.setScale({1, 1});
	sprite.setPosition({window.getSize().x - texSize.x - 5.f, window.getSize().y - texSize.y - 5.f});

	window.draw(sprite);
	window.setView(previousView);
}
