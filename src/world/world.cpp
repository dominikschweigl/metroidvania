#include "world.h"
#include <iostream>
#include <algorithm>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Level
{
    int width{};
    int height{};
    std::vector<int> tiles;
};

#include <fstream>

std::string readFile(const std::string& path)
{
    std::ifstream file(path);
    return std::string(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}

World::World()
{
    // Constructor - tiles will be populated via loadFromGrid
}

void World::loadFromGrid(const std::vector<std::vector<int>>& grid)
{
    tiles.clear();
	tiles.resize(grid.size());

    for (size_t y = 0; y < grid.size(); ++y) {
        tiles[y].clear();
        for (size_t x = 0; x < grid[y].size(); ++x) {
            int tileType = grid[y][x];
			Tile tile;
			tile.position = {x * TILE_SIZE, y * TILE_SIZE};
			tile.size = {TILE_SIZE, TILE_SIZE};
			tile.isSolid = tileType > 0;
			tile.textureId = tileType;
			tiles[y].push_back(tile);
        }
    }
}

void World::loadFromJson(const std::string& filename)
{
    std::string data = readFile(filename);

    json j = json::parse(data);

    Level level;
    level.width = j["width"];
    level.height = j["height"];
    level.tiles = j["tiles"].get<std::vector<int>>();

    tiles.resize(level.height);

    for (int y = 0; y < level.height; y++)
    {
        tiles[y].resize(level.width);

        for (int x = 0; x < level.width; x++)
        {
            int value = level.tiles[y * level.width + x];

            Tile& tile = tiles[y][x];
            tile.position = {x * TILE_SIZE, y * TILE_SIZE};
            tile.size = {TILE_SIZE, TILE_SIZE};
            tile.isSolid = (value > 0);
            tile.textureId = value;
        }
    }
}

const std::optional<const World::Tile*> World::getTileAtCoordinate(const sf::Vector2f& worldPos) const
{
    int x = static_cast<int>(worldPos.x / TILE_SIZE);
    int y = static_cast<int>(worldPos.y / TILE_SIZE);

    if (y < 0 || y >= (int)tiles.size())
        return std::nullopt;

    if (x < 0 || x >= (int)tiles[y].size())
        return std::nullopt;

    return &tiles[y][x];
}

bool World::isSolidAtRect(const sf::FloatRect& rect) const
{
	std::vector<std::vector<const World::Tile*>> tilesInRect = World::getTilesAtRect(rect);

	for (const std::vector<const World::Tile*>& tileRow : tilesInRect)
	{
		for (const World::Tile* tile : tileRow)
		{
			if (tile->isSolid && tile->getBounds().findIntersection(rect).has_value())
				return true;
		}
	}

	return false;
}

std::vector<std::vector<const World::Tile*>> World::getTilesAtRect(const sf::FloatRect& rect) const
{
    std::vector<std::vector<const Tile*>> result;

    if (tiles.empty() || tiles[0].empty())
        return result;

    int left   = static_cast<int>(rect.position.x / TILE_SIZE);
    int right  = static_cast<int>((rect.position.x + rect.size.x) / TILE_SIZE);
    int top    = static_cast<int>(rect.position.y / TILE_SIZE);
    int bottom = static_cast<int>((rect.position.y + rect.size.y) / TILE_SIZE);

    int maxY = (int)tiles.size() - 1;
    int maxX = (int)tiles[0].size() - 1;

    left   = std::clamp(left, 0, maxX);
    right  = std::clamp(right, 0, maxX);
    top    = std::clamp(top, 0, maxY);
    bottom = std::clamp(bottom, 0, maxY);

    for (int y = top; y <= bottom; ++y)
    {
        result.push_back(std::vector<const Tile*>());
        for (int x = left; x <= right; ++x)
        {
            result.back().push_back(&tiles[y][x]);
        }
    }

    return result;
}

void World::draw(sf::RenderWindow& window, const sf::View& view) const
{
	sf::Vector2f center = view.getCenter();
	sf::Vector2f size = view.getSize();

	sf::FloatRect viewRect(
		{center.x - size.x * 0.5f, center.y - size.y * 0.5f},
		{size.x, size.y}
	);

	std::vector<std::vector<const Tile*>> visibleTiles = World::getTilesAtRect(viewRect);

    for (const std::vector<const Tile*> tileRow : visibleTiles)
    {
        for (const Tile* tile : tileRow)
        {
            sf::RectangleShape shape(tile->size);
            shape.setPosition(tile->position);
			if (tile->isSolid){
				int ix = static_cast<int>(tile->position.x / TILE_SIZE);
				int iy = static_cast<int>(tile->position.y / TILE_SIZE);
				if ((ix + iy) % 2 == 0) {
					shape.setFillColor(sf::Color(100, 100, 100));
				} else {
					shape.setFillColor(sf::Color(150, 150, 150)); // Slightly lighter color for non-aligned tiles (for visual variety)
				}
			} else {
				shape.setFillColor(sf::Color(135, 206, 235)); // Lighter color for non-solid tiles
			}

			window.draw(shape);
		}
    }
}
