#include "minimap.h"
#include "../core/asset_manager.h"
#include <iostream>

void MiniMap::draw(sf::RenderWindow &window, const sf::Vector2f playerPos, Room &room)
{
	const sf::View previousView = window.getView();
	window.setView(sf::View(sf::FloatRect({0.f, 0.f}, sf::Vector2f(window.getSize()))));

	const sf::Texture &minimap = AssetManager::getInstance().getTexture(
	    room.world_index < minimaps.size() ? minimaps.at(room.world_index) : minimaps.at(0));
	const sf::Vector2u texSize = minimap.getSize();
	sf::Sprite sprite(minimap);
	sprite.setScale({1, 1});
	// Right Bottom
	// sf::Vector2f minimapPos =
	//     sf::Vector2f({window.getSize().x - texSize.x - 5.f, window.getSize().y - texSize.y + 5.f});
	sf::Vector2f minimapPos = sf::Vector2f({5.f, 5.f});
	sprite.setPosition(minimapPos);

	sf::CircleShape playerCircle;
	playerCircle.setRadius(4.f);
	playerCircle.setFillColor(sf::Color(255, 0, 0));

	sf::Vector2f relativePlayerPos =
	    sf::Vector2f({playerPos.x / (room.width * World::TILE_SIZE), playerPos.y / (room.height * World::TILE_SIZE)});

	const sf::FloatRect &mapRoom = calcRelativePositionInMiniMapByPixels(room.minimap_pixel_rect, room.world_index);

	playerCircle.setPosition(
	    {minimapPos.x + (mapRoom.position.x + relativePlayerPos.x * mapRoom.size.x) * (texSize.x - 5.f),
	     minimapPos.y + (mapRoom.position.y + relativePlayerPos.y * mapRoom.size.y) * (texSize.y - 5.f)});

	window.draw(sprite);
	window.draw(playerCircle);
	window.setView(previousView);
}
