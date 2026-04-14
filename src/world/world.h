#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <optional>

class World {
public:
    struct Tile {
        sf::Vector2f position;
        sf::Vector2f size;
        bool isSolid;
        int textureId; // For future texture mapping

        sf::FloatRect getBounds() const {
            return sf::FloatRect(position, size);
        }
    };

    static constexpr float TILE_SIZE = 32.f;

    World();
    ~World() = default;

    // Build world from a simple grid layout
    void loadFromGrid(const std::vector<std::vector<int>>& grid);
	void loadFromJson(const std::string& filename);

	// Just a helper for retrieving tile at a specific world coordinate
	const std::optional<const Tile*> getTileAtCoordinate(const sf::Vector2f& worldPos) const;
    // Get all tiles within a rectangle (for collision checks)
	std::vector<std::vector<const Tile*>> getTilesAtRect(const sf::FloatRect& rect) const;
	// Return true if any solid tile intersects with the given rectangle
	bool isSolidAtRect(const sf::FloatRect& rect) const;

    // Render all visible tiles
    void draw(sf::RenderWindow& window, const sf::View& view) const;

private:
    std::vector<std::vector<Tile>> tiles;

    static float getRectLeft(const sf::FloatRect& rect) {
        return rect.position.x;
    }
    static float getRectTop(const sf::FloatRect& rect) {
        return rect.position.y;
    }
    static float getRectRight(const sf::FloatRect& rect) {
        return rect.position.x + rect.size.x;
    }
    static float getRectBottom(const sf::FloatRect& rect) {
        return rect.position.y + rect.size.y;
    }
};
