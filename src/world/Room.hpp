#pragma once
#include "../entities/enemies/base_enemy.h"
#include "../items/world_item.h"
#include "../ui/interaction_indicator.h"
#include "../utils/EnemyFactory.hpp"
#include <SFML/Graphics.hpp>
#include <functional>
#include <nlohmann/json.hpp>
#include <optional>
#include <string_view>
#include <tileson.hpp>
#include <unordered_map>
#include <vector>

using json = nlohmann::json;

struct SavePoint {
	sf::FloatRect bounds;
	InteractionIndicator indicator;
};

struct Door {
	std::string targetRoomId;
	int targetSpawnIdx;
	sf::FloatRect bounds;
	InteractionIndicator indicator;
};

struct ImageLayer {
	std::shared_ptr<sf::Texture> texture;
	tson::Vector2f position;
	tson::Vector2f parallax;
	bool repeatX = false;
};

struct Room {
	int width{}, height{};
	std::shared_ptr<tson::Map> map;
	std::vector<std::vector<bool>> solidGrid; // used for test maps without real tilemaps, indexed by [y][x]

	std::vector<sf::Vector2f> playerSpawns;
	Direction playerSpawnDirection = Direction::Right;
	std::vector<Door> doors;
	std::vector<SavePoint> savePoints;
	std::vector<std::unique_ptr<BaseEnemy>> enemies_;
	std::vector<std::unique_ptr<WorldItem>> items_;

	std::vector<ImageLayer> backgroundLayers;

	bool needsToClearAllEnemies = false;

	Room() = default;
	Room(Room &&) = default;
	Room &operator=(Room &&) = default;
	Room(const Room &) = delete;
	Room &operator=(const Room &) = default;

	void appendItem(std::unique_ptr<WorldItem> &newItem) { items_.push_back(std::move(newItem)); }

	bool isTouchingSavepoint(const sf::FloatRect &entityBounds) const
	{
		for (size_t i = 0; i < savePoints.size(); ++i) {
			if (savePoints[i].bounds.findIntersection(entityBounds)) {
				return true;
			}
		}
		return false;
	}

	std::optional<std::pair<std::string, int>> getTouchingDoorTargetRoom(const sf::FloatRect &entityBounds) const
	{
		for (const Door &door : doors) {
			if (door.bounds.findIntersection(entityBounds)) {
				return std::make_optional(std::make_pair(door.targetRoomId, door.targetSpawnIdx));
			}
		}
		return std::nullopt;
	}

	std::optional<sf::FloatRect> getTouchingInteractableBounds(const sf::FloatRect &entityBounds) const
	{
		for (const Door &door : doors) {
			if (door.bounds.findIntersection(entityBounds)) {
				return door.bounds;
			}
		}
		for (const SavePoint &savePoint : savePoints) {
			if (savePoint.bounds.findIntersection(entityBounds)) {
				return savePoint.bounds;
			}
		}
		return std::nullopt;
	}

	void update(float deltaTime, const World &world)
	{
		enemies_.erase(std::remove_if(enemies_.begin(), enemies_.end(), [](const auto &e) { return !e->isAlive(); }),
		               enemies_.end());
		items_.erase(std::remove_if(items_.begin(), items_.end(), [](const auto &i) { return i->isCollected(); }),
		             items_.end());
		updateInteractionIndicators(deltaTime);
	}

	void draw(sf::RenderWindow &window, const sf::FloatRect playerBounds, const float playerX) const
	{
		drawInteractionIndicators(window, playerBounds, playerX);
	}

	bool isAllowedLeaving()
	{
		if (!needsToClearAllEnemies)
			return true;

		for (const auto &enemy : enemies_) {
			if (enemy->isAlive()) {
				return false;
			}
		}
		return true;
	}

	json serialize() const
	{
		json j;

		j["enemies"] = json::array();

		for (const auto &e : enemies_) {
			j["enemies"].push_back(e->serialize());
		}

		j["items"] = json::array();

		for (const auto &i : items_) {
			j["items"].push_back(i->serialize());
		}

		return j;
	}

	void deserialize(const json &j)
	{
		width = j.value("width", width);
		height = j.value("height", height);

		if (j.contains("enemies")) {
			enemies_.clear();
			for (const auto &e : j["enemies"]) {
				enemies_.push_back(EnemyFactory::create(e));
			}
		}

		if (j.contains("items")) {
			items_.clear();

			for (const auto &i : j["items"]) {
				items_.push_back(WorldItem::deserialize(i));
			}
		}
	}

  private:
	void updateInteractionIndicators(const float deltaTime)
	{
		for (Door &door : doors) {
			door.indicator.update(deltaTime);
		}
		for (SavePoint &savePoint : savePoints) {
			savePoint.indicator.update(deltaTime);
		}
	}

	void drawInteractionIndicators(sf::RenderWindow &window, const sf::FloatRect playerBounds,
	                               const float playerX) const
	{
		for (const Door &door : doors) {
			if (door.bounds.findIntersection(playerBounds)) {
				door.indicator.draw(window, door.bounds, playerX);
				return;
			}
		}
		for (const SavePoint &savePoint : savePoints) {
			if (savePoint.bounds.findIntersection(playerBounds)) {
				savePoint.indicator.draw(window, savePoint.bounds, playerX);
				return;
			}
		}
	}
};
