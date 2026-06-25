#pragma once
#include "../core/asset_manager.h"
#include "../entities/enemies/base_enemy.h"
#include "../items/world_item.h"
#include "../utils/ItemFactory.hpp"
#include "Room.hpp"
#include <SFML/Graphics.hpp>
#include <functional>
#include <nlohmann/json.hpp>
#include <optional>
#include <unordered_map>
#include <vector>

using json = nlohmann::json;

class World {
  public:
	static constexpr float TILE_SIZE = 32.f;

	std::string worldName = "world";

	~World() = default;
	World(const std::string worldName);
	World(World &&) = default;
	World &operator=(World &&) = default;
	World(const World &) = delete;
	World &operator=(const World &) = delete;

	// New room-based methods
	void loadRoom(const std::string &roomId, const std::string &tmjFile);
	void setCurrentRoom(const std::string &roomId);
	const std::string &getCurrentRoomId() const { return currentRoomId; }
	Room *getCurrentRoom();
	[[nodiscard]] float getWorldHeight();

	void loadTileset();
	void loadTilesets(tson::Map &map);

	// Just a helper for retrieving tile at a specific world coordinate
	tson::Tile *getTileAtCoordinate(const sf::Vector2f &worldPos, const std::string &layerName) const;
	// Return true if any solid tile intersects with the given rectangle
	bool isSolidAtRect(const sf::FloatRect &rect) const;

	void loadFromGrid(const std::vector<std::vector<int>> &grid);

	// Save the current world state
	void saveWorldData(Player &player);
	// Load the last saved world state
	void loadWorldData(Player &player);

	// Render all visible tiles and interaction indicators
	void draw(sf::RenderWindow &window, const sf::View &view, sf::FloatRect playerBounds) const;

	void update(float deltaTime, sf::FloatRect playerBounds);

	Door *getTouchingDoor(const sf::FloatRect &entityBounds) { return getCurrentRoom()->getTouchingDoor(entityBounds); }

	bool isTouchingSavepoint(const sf::FloatRect &entityBounds)
	{
		return getCurrentRoom()->isTouchingSavepoint(entityBounds);
	}

  private:
	std::unordered_map<std::string, Room> rooms;
	std::string currentRoomId;
	std::unordered_map<int, const sf::Texture> tileTextures;

	static float getRectLeft(const sf::FloatRect &rect) { return rect.position.x; }
	static float getRectTop(const sf::FloatRect &rect) { return rect.position.y; }
	static float getRectRight(const sf::FloatRect &rect) { return rect.position.x + rect.size.x; }
	static float getRectBottom(const sf::FloatRect &rect) { return rect.position.y + rect.size.y; }
};
