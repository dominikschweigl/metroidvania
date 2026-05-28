#pragma once
#include "../core/asset_manager.h"
#include "Room.hpp"
#include <SFML/Graphics.hpp>
#include <functional>
#include <optional>
#include <unordered_map>
#include <vector>

class World {
  public:
	static constexpr float TILE_SIZE = 32.f;
	~World() = default;
	World() = default;
	World(World &&) = default;
	World &operator=(World &&) = default;
	World(const World &) = delete;
	World &operator=(const World &) = delete;

	// New room-based methods
	void loadRoom(const std::string &roomId, const std::string &tmjFile);
	void setCurrentRoom(const std::string &roomId);
	const std::string &getCurrentRoomId() const { return currentRoomId; }
	const Room *getCurrentRoom() const;
	[[nodiscard]] float getWorldHeight() const;

	void loadTileset();
	void loadTilesets(tson::Map &map);

	// Just a helper for retrieving tile at a specific world coordinate
	tson::Tile *getTileAtCoordinate(const sf::Vector2f &worldPos, const std::string &layerName) const;
	// Return true if any solid tile intersects with the given rectangle
	bool isSolidAtRect(const sf::FloatRect &rect) const;

	void loadFromGrid(const std::vector<std::vector<int>> &grid);

	// Render all visible tiles
	void draw(sf::RenderWindow &window, const sf::View &view) const;

	std::string getTouchingDoorTargetRoom(const sf::FloatRect &entityBounds)
	{
		return getCurrentRoom()->getTouchingDoorTargetRoom(entityBounds);
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
