#pragma once
#include "../entities/enemies/base_enemy.h"
#include "../items/world_item.h"
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

	std::vector<std::unique_ptr<BaseEnemy>> enemies_;
	std::vector<std::unique_ptr<WorldItem>> items_;

	Room() = default;
	Room(Room &&) = default;
	Room &operator=(Room &&) = default;
	Room(const Room &) = default;
	Room &operator=(const Room &) = default;

	void appendItem(std::unique_ptr<WorldItem> &newItem) { items_.push_back(std::move(newItem)); }

	std::string getTouchingDoorTargetRoom(const sf::FloatRect &entityBounds) const
	{
		for (const Door &door : doors) {
			if (door.bounds.findIntersection(entityBounds)) {
				return door.targetRoomId;
			}
		}
		return "";
	}

	void update(float deltaTime, const World &world)
	{
		enemies_.erase(std::remove_if(enemies_.begin(), enemies_.end(), [](const auto &e) { return !e->isAlive(); }),
		               enemies_.end());
		items_.erase(std::remove_if(items_.begin(), items_.end(), [](const auto &i) { return i->isCollected(); }),
		             items_.end());
	}
};
