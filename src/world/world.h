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
	static constexpr float UPWARD_CLIP = 10.f; // Amount to clip from player's bottom when checking ground collision, to allow smoother landing on edges

    World();
    ~World() = default;

    // Build world from a simple grid layout
    // Grid format: 0 = empty, >0 = solid tile with texture ID
    void loadFromGrid(const std::vector<std::vector<int>>& grid);

    // Check collision with world and return the ground Y position
    // Returns the Y coordinate of the highest solid surface below the player
    std::optional<float> getGroundYAt(const sf::FloatRect& playerBounds) const;
	std::optional<float> willCollideWithWall(const sf::FloatRect& playerBounds, const sf::Vector2f& velocity, float deltaTime, const World& world) const;

    // Get all tiles within view for efficient rendering
    std::vector<std::vector<const Tile*>> getTilesInView(const sf::View& view) const;

    // Render all visible tiles
    void draw(sf::RenderWindow& window, const sf::View& view) const;

    // Utility: Check if a point is inside a solid tile
    bool isPointSolid(const sf::Vector2f& point) const;

    // Get tile at grid coordinates (returns nullptr if out of bounds or empty)
    const Tile* getTileAt(int gridX, int gridY) const;

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
