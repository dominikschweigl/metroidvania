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

struct Room {
	int width{}, height{};
	std::shared_ptr<tson::Map> map;
	std::vector<std::vector<bool>> solidGrid; // used when map is null

	sf::Vector2f playerSpawn{};
	std::vector<sf::Vector2f> raceConditionSpawns;
	std::vector<Door> doors;

	Room() = default;
	Room(Room &&) = default;
	Room &operator=(Room &&) = default;
	Room(const Room &) = default;
	Room &operator=(const Room &) = default;

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
