#pragma once
#include "../world/world.h"
#include <SFML/Graphics.hpp>
#include <vector>

// Top-left HUD health indicator.
class MiniMap {
  public:
	static constexpr float MINIMAP_HEIGHT = 287.f;
	static constexpr float MINIMAP_WIDTH = 287.f;

	// This function is used to calculate the relative position and size of a room in the minimap by specifying for
	// example the pixel coordinates of the top_left and the bottom_right corners.
	static sf::FloatRect calcRelativePositionInMiniMapByPixels(sf::Vector2f top_left, sf::Vector2f bottom_right)
	{
		return sf::FloatRect({top_left.x / MiniMap::MINIMAP_WIDTH, top_left.y / MiniMap::MINIMAP_HEIGHT},
		                     {(bottom_right.x - top_left.x) / MiniMap::MINIMAP_HEIGHT,
		                      (bottom_right.y - top_left.y) / MiniMap::MINIMAP_HEIGHT});
	}
	// This is needed to calculate the position of the player in the minimap.
	const std::unordered_map<std::string, sf::FloatRect> room_in_minimap{
	    {"start_room", calcRelativePositionInMiniMapByPixels({53, 213}, {132, 249})},
	    {"1", calcRelativePositionInMiniMapByPixels({141, 176}, {192, 217})},
	    {"2", calcRelativePositionInMiniMapByPixels({67, 143}, {132, 180})},
	    {"3", calcRelativePositionInMiniMapByPixels({141, 134}, {172, 147})},
	    {"4", calcRelativePositionInMiniMapByPixels({45, 72}, {58, 150})},
	    {"5", calcRelativePositionInMiniMapByPixels({181, 100}, {232, 147})},
	    {"6", calcRelativePositionInMiniMapByPixels({241, 85}, {258, 102})},
	    {"7", calcRelativePositionInMiniMapByPixels({68, 47}, {172, 105})},
	    {"8", calcRelativePositionInMiniMapByPixels({28, 23}, {88, 64})},
	};

	MiniMap() = default;
	~MiniMap() = default;
	MiniMap(const MiniMap &) = delete;
	MiniMap &operator=(const MiniMap &) = delete;
	MiniMap(MiniMap &&) = delete;
	MiniMap &operator=(MiniMap &&) = delete;

	// Draws the minimap in screen-space using the window's default view.
	void draw(sf::RenderWindow &window, const sf::Vector2f playerPos, const std::string room_id, Room &room);
};
