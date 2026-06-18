#include "minimap.h"
#include "../core/asset_manager.h"
#include <iostream>

void MiniMap::draw(sf::RenderWindow &window, const sf::Vector2f playerPos, const std::string room_id, Room &room)
{
	const sf::View previousView = window.getView();
	window.setView(sf::View(sf::FloatRect({0.f, 0.f}, sf::Vector2f(window.getSize()))));

	const sf::Texture &minimap = AssetManager::getInstance().getTexture(TextureAsset::MINIMAP);
	const sf::Vector2u texSize = minimap.getSize();
	sf::Sprite sprite(minimap);
	sprite.setScale({1, 1});
	sf::Vector2f minimapPos =
	    sf::Vector2f({window.getSize().x - texSize.x - 5.f, window.getSize().y - texSize.y - 5.f});
	sprite.setPosition(minimapPos);
	// sprite.setPosition({0, 0});

	sf::CircleShape playerCircle;
	playerCircle.setRadius(4.f);
	playerCircle.setFillColor(sf::Color(255, 0, 0));
	if (room_in_minimap.contains(room_id)) {
		sf::Vector2f relativePlayerPos = sf::Vector2f(
		    {playerPos.x / (room.width * World::TILE_SIZE), playerPos.y / (room.height * World::TILE_SIZE)});
		// playerCircle.setPosition(
		//     {minimapPos.x + (room_in_minimap.at(room_id).position.x + relativePlayerPos.x) * (texSize.x - 5.f),
		//      minimapPos.y + (room_in_minimap.at(room_id).position.y + relativePlayerPos.y) * (texSize.y - 5.f)});

		const auto &mapRoom = room_in_minimap.at(room_id);

		playerCircle.setPosition(
		    {minimapPos.x + (mapRoom.position.x + relativePlayerPos.x * mapRoom.size.x) * (texSize.x - 5.f),
		     minimapPos.y + (mapRoom.position.y + relativePlayerPos.y * mapRoom.size.y) * (texSize.y - 5.f)});

		// playerCircle.setPosition(
		//     {minimapPos.x
		//          + (room_in_minimap.at(room_id).position.x + relativePlayerPos.x +
		//          room_in_minimap.at(room_id).size.x)
		//                * (texSize.x - 5.f),
		//      minimapPos.y
		//          + (room_in_minimap.at(room_id).position.y + relativePlayerPos.y +
		//          room_in_minimap.at(room_id).size.y)
		//                * (texSize.y - 5.f)});
	} else {
		std::cout << "Roomid: " << room_id << " does not exist" << std::endl;
		playerCircle.setPosition({minimapPos.x, minimapPos.y});
	}

	window.draw(sprite);
	window.draw(playerCircle);
	window.setView(previousView);
}
