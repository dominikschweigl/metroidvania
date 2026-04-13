#include "world.h"
#include <iostream>




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
            if (tileType > 0) {
                Tile tile;
                tile.position = {x * TILE_SIZE, y * TILE_SIZE};
                tile.size = {TILE_SIZE, TILE_SIZE};
                tile.isSolid = true;
                tile.textureId = tileType;
                tiles[y].push_back(tile);
            }
        }
    }
}

std::optional<float> World::getGroundYAt(const sf::FloatRect& playerBounds) const
{
    std::optional<float> groundY;

    for (const auto& row : getTilesInView(sf::View(playerBounds.getCenter(), playerBounds.size * 2.f))) {
        for (const auto& tile : row) {
            if (!tile->isSolid) continue;

			sf::FloatRect tileBounds = tile->getBounds();

			if (getRectRight(playerBounds) > getRectLeft(tileBounds) &&
				getRectLeft(playerBounds) < getRectRight(tileBounds)) {

				if (getRectBottom(playerBounds) - UPWARD_CLIP < getRectTop(tileBounds) && getRectBottom(playerBounds) >= getRectTop(tileBounds)) {

					float tileTop = getRectTop(tileBounds);
					if (!groundY.has_value() || tileTop < groundY.value()) {
						groundY = tileTop;
					}
				}
			}
		}
    }
    return groundY;
}

std::vector<std::vector<const World::Tile*>> World::getTilesInView(const sf::View& view) const
{
    std::vector<std::vector<const Tile*>> visibleTiles;

    sf::Vector2f center = view.getCenter();
    sf::Vector2f size = view.getSize();
    sf::FloatRect viewBounds(
        {center.x - size.x / 2.f, center.y - size.y / 2.f},
        {size.x, size.y}
    );

    for (const auto& row : tiles) {
        std::vector<const Tile*> visibleTilesInRow;
        for (const auto& tile : row) {
            if (viewBounds.findIntersection(tile.getBounds())) {
                visibleTilesInRow.push_back(&tile);
            }
        }
        if (!visibleTilesInRow.empty()) {
            visibleTiles.push_back(visibleTilesInRow);
        }
    }

    return visibleTiles;
}

std::optional<float> World::willCollideWithWall(const sf::FloatRect & playerBounds, const sf::Vector2f & velocity, float deltaTime, const World & world) const
{
    if (velocity.x == 0.f)
        return std::optional<float>();

    float dx = velocity.x * deltaTime;
    float dy = velocity.y * deltaTime;

    // Check points along the right edge
    sf::Vector2f topLeft     = {playerBounds.position.x + dx, playerBounds.position.y + dy + 1.f};
    sf::Vector2f topRight     = {playerBounds.position.x + playerBounds.size.x + dx, playerBounds.position.y + dy + 1.f};
    sf::Vector2f bottomLeft  = {playerBounds.position.x + dx, playerBounds.position.y + dy + playerBounds.size.y - UPWARD_CLIP};
    sf::Vector2f bottomRight  = {playerBounds.position.x + playerBounds.size.x + dx, playerBounds.position.y + dy + playerBounds.size.y - UPWARD_CLIP};

    if (world.isPointSolid(topRight) ||
        world.isPointSolid(topLeft) ||
        world.isPointSolid(bottomRight) ||
        world.isPointSolid(bottomLeft)) {
        return 1.f;
    }

    return std::nullopt;
}

void World::draw(sf::RenderWindow& window, const sf::View& view) const
{
    auto visibleTiles = getTilesInView(view);

    for (const auto& row : visibleTiles) {
        for (const auto* tile : row) {
            sf::RectangleShape shape(tile->size);
            shape.setPosition(tile->position);

			int ix = static_cast<int>(tile->position.x / TILE_SIZE);
			int iy = static_cast<int>(tile->position.y / TILE_SIZE);
			if ((ix + iy) % 2 == 0) {
				shape.setFillColor({100, 100, 100});
			} else {
				shape.setFillColor({120, 120, 120});
			}

			// shape.setOutlineColor({50, 50, 50});
			// shape.setOutlineThickness(1.f);

			window.draw(shape);
		}
    }
}

bool World::isPointSolid(const sf::Vector2f& point) const
{
    for (const auto& tileRow : tiles) {
        for (const auto& tile : tileRow) {
            if (!tile.isSolid) continue;

			sf::FloatRect bounds = tile.getBounds();
			if (bounds.contains(point)) {
				return true;
			}
		}
    }
    return false;
}

const World::Tile* World::getTileAt(int gridX, int gridY) const
{
    sf::Vector2f worldPos = {gridX * TILE_SIZE, gridY * TILE_SIZE};

    for (const auto& tileRow : tiles) {
        for (const auto& tile : tileRow) {
            if (tile.position == worldPos) {
                return &tile;
            }
        }
    }

    return nullptr;
}
