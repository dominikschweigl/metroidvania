#pragma once
#include "../entities/enemies/base_enemy.h"
#include "../items/world_item.h"
#include "../ui/interaction_indicator.h"
#include "../utils/EnemyFactory.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <tileson.hpp>
#include <vector>

using json = nlohmann::json;

struct Interactable {
	sf::FloatRect bounds;
	InteractionIndicator indicator;
};

struct SavePoint : Interactable {};

struct Door : Interactable {
	std::string targetRoomId;
	int targetSpawnIdx = 0;
	bool needsToClearAllEnemies = false;
	bool locked = false;
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
	std::vector<std::vector<bool>> solidGrid;

	std::vector<sf::Vector2f> playerSpawns;
	Direction playerSpawnDirection = Direction::Right;
	std::vector<Door> doors;
	std::vector<SavePoint> savePoints;
	std::vector<std::unique_ptr<BaseEnemy>> enemies_;
	std::vector<std::unique_ptr<WorldItem>> items_;

	std::vector<ImageLayer> backgroundLayers;

	bool world_index = 0;
	sf::FloatRect minimap_pixel_rect{};

	Room() = default;
	Room(Room &&) = default;
	Room &operator=(Room &&) = default;
	Room(const Room &) = delete;
	Room &operator=(const Room &) = default;

	void appendItem(std::unique_ptr<WorldItem> &newItem);

	bool isTouchingSavepoint(const sf::FloatRect &entityBounds) const;
	Door *getTouchingDoor(const sf::FloatRect &entityBounds);

	void update(float deltaTime);
	void draw(sf::RenderWindow &window, const sf::FloatRect playerBounds, float playerX) const;

	json serialize() const;
	void deserialize(const json &j);

  private:
	void updateInteractionIndicators(float deltaTime);
	void drawInteractionIndicators(sf::RenderWindow &window, sf::FloatRect playerBounds, float playerX) const;
};
