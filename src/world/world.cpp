#include "world.h"
#include <iostream>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <tinyxml2.h>

using namespace tinyxml2;
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

void World::loadTileset()
{
    std::vector<std::string> paths = {
        "assets/images/tiles/pixil-frame-0.png",
        "assets/images/tiles/pixil-frame-1.png",
        "assets/images/tiles/pixil-frame-2.png",
        "assets/images/tiles/pixil-frame-3.png"
    };

    for (int i = 0; i < paths.size(); i++)
    {
        sf::Texture tex;
        bool loaded = tex.loadFromFile(paths[i]);
        if (!loaded) {
            std::cerr << "Failed to load texture from " << paths[i] << std::endl;
        }
        tileTextures[i] = tex;
    }
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

TiledMap loadMap(const std::string& file)
{
    std::ifstream f(file);
    json j;
    f >> j;

    TiledMap map;

    map.width = j["width"];
    map.height = j["height"];
    map.tilewidth = j["tilewidth"];
    map.tileheight = j["tileheight"];

    for (auto& layer : j["layers"])
    {
        if (layer["type"] != "tilelayer") continue;

        TiledLayer l;
        l.width = layer["width"];
        l.height = layer["height"];
        l.data = layer["data"].get<std::vector<int>>();

        map.layers.push_back(std::move(l));
    }

    return map;
}

void World::loadFromTMJ(const std::string& file)
{
    TiledMap map = loadMap(file);

    const auto& layer = map.layers[0];

    tiles.resize(map.height);

    for (int y = 0; y < map.height; y++)
    {
        tiles[y].resize(map.width);

        for (int x = 0; x < map.width; x++)
        {
            int value = layer.data[y * map.width + x];

            Tile& tile = tiles[y][x];

            tile.position = {
                (float) x * map.tilewidth,
                (float) y * map.tileheight
            };

            tile.size = {
                (float)map.tilewidth,
                (float)map.tileheight
            };

            tile.isSolid = (value != 0);
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

    for (auto& row : visibleTiles)
    {
        for (auto& tile : row)
        {
            if (tile->textureId == 0) continue;

            sf::RectangleShape shape(tile->size);
            shape.setPosition(tile->position);

            auto it = tileTextures.find(std::max(0, tile->textureId - 1));
            if (it != tileTextures.end())
            {
                shape.setTexture(&it->second);
            } else {
				shape.setFillColor(sf::Color(255, 0, 255)); // Magenta for missing texture
			}

            window.draw(shape);
        }
    }
}
