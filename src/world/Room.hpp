#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <optional>
#include <string_view>
#include <tileson.hpp>
#include <unordered_map>
#include <vector>

struct Door {
	std::string targetRoomId;
	sf::FloatRect bounds;
};

class Room {
  public:
	// unique_ptr deletes copy — explicitly default move Room() = default;
	Room() = default;
	Room(Room &&) = default;
	Room &operator=(Room &&) = default;

	// explicitly delete copy
	Room(const Room &) = delete;
	Room &operator=(const Room &) = delete;

	int width{}, height{};
	std::unique_ptr<tson::Map> map;

	sf::Vector2f playerSpawn{};
	std::vector<sf::Vector2i> raceConditionSpawns;
	std::vector<Door> doors; // define Door as needed

	std::string getTouchingDoorTargetRoom(const sf::FloatRect &entityBounds) const
	{
		for (const Door &door : doors) {
			if (door.bounds.findIntersection(entityBounds)) {
				return door.targetRoomId;
			}
		}
		return "";
	}
};
