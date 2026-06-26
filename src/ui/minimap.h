#pragma once
#include "../world/world.h"
#include <SFML/Graphics.hpp>
#include <vector>

// Top-left HUD health indicator.
class MiniMap {
  public:
	// This function is used to calculate the relative position and size of a room in the minimap by specifying for
	// example the pixel coordinates of the top_left and the bottom_right corners.
	const std::vector<sf::Vector2f> minimap_sizes{{287.f, 287.f}, {256.f, 256.f}};

	sf::FloatRect calcRelativePositionInMiniMapByPixels(sf::FloatRect rect, size_t world_index)
	{
		const auto &size = minimap_sizes.at(world_index);

		return sf::FloatRect({rect.position.x / size.x, rect.position.y / size.y},
		                     {rect.size.x / size.x, rect.size.y / size.y});
	}

	const std::vector<TextureAsset> minimaps{
	    TextureAsset::FIRST_AREA_MINIMAP,
	    TextureAsset::SECOND_AREA_MINIMAP,
	};

	MiniMap() = default;
	~MiniMap() = default;
	MiniMap(const MiniMap &) = delete;
	MiniMap &operator=(const MiniMap &) = delete;
	MiniMap(MiniMap &&) = delete;
	MiniMap &operator=(MiniMap &&) = delete;

	// Draws the minimap in screen-space using the window's default view.
	void draw(sf::RenderWindow &window, const sf::Vector2f playerPos, Room &room);
};
