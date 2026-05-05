#pragma once
#include <SFML/Graphics.hpp>
#include <optional>
#include <unordered_map>
#include <vector>

struct TileDef {
	int id;
	std::string imagePath;
};

struct TiledLayer {
	std::vector<int> data;
	int width{};
	int height{};
};

struct TiledMap {
	int width{};
	int height{};
	int tilewidth{};
	int tileheight{};
	std::vector<TiledLayer> layers;
};

class World {
  public:
	struct Tile {
		sf::Vector2f position;
		sf::Vector2f size;
		bool isSolid;
		int textureId;

		sf::FloatRect getBounds() const {
			return sf::FloatRect(position, size);
		}
	};

	struct Room {
		std::vector<std::vector<Tile>> tiles;
		int width{};
		int height{};
	};

	static constexpr float TILE_SIZE = 32.f;

	World();
	~World() = default;

	// Build world from a simple grid layout (deprecated, use rooms)
	void loadFromGrid(const std::vector<std::vector<int>> &grid);
	void loadFromJson(const std::string &filename);
	void loadFromTMJ(const std::string &file);

	TiledMap loadMap(const std::string &file);

	// New room-based methods
	void loadRoom(const std::string &roomId, const std::string &tmjFile);
	void setCurrentRoom(const std::string &roomId);
	const std::string &getCurrentRoomId() const { return currentRoomId; }

	void loadTileset();

	// Just a helper for retrieving tile at a specific world coordinate
	const std::optional<const Tile *>
	getTileAtCoordinate(const sf::Vector2f &worldPos) const;
	// Get all tiles within a rectangle (for collision checks)
	std::vector<std::vector<const Tile *>>
	getTilesAtRect(const sf::FloatRect &rect) const;
	// Return true if any solid tile intersects with the given rectangle
	bool isSolidAtRect(const sf::FloatRect &rect) const;

	// Render all visible tiles
	void draw(sf::RenderWindow &window, const sf::View &view) const;

  private:
	std::unordered_map<std::string, Room> rooms;
	std::string currentRoomId;
	std::unordered_map<int, sf::Texture> tileTextures;

	static float getRectLeft(const sf::FloatRect &rect) {
		return rect.position.x;
	}
	static float getRectTop(const sf::FloatRect &rect) {
		return rect.position.y;
	}
	static float getRectRight(const sf::FloatRect &rect) {
		return rect.position.x + rect.size.x;
	}
	static float getRectBottom(const sf::FloatRect &rect) {
		return rect.position.y + rect.size.y;
	}
};
