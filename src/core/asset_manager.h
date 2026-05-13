#pragma once
#include <SFML/Graphics.hpp>
#include <string_view>
#include <unordered_map>

enum TextureAsset {
	// Player
	PLAYER_IDLE_HAT,
	PLAYER_IDLE_LOWER_BODY,
	PLAYER_WALK_HAT,
	PLAYER_WALK_LOWER_BODY,
	PLAYER_WALK_UPPER_BODY,
	PLAYER_RUN_HAT,
	PLAYER_RUN_LOWER_BODY,
	PLAYER_RUN_UPPER_BODY,
	PLAYER_JUMP,
	PLAYER_ATTACK_SWING_UPPER_BODY,
	PLAYER_ATTACK_OVERHEAD_UPPER_BODY,

	// Race-condition slime
	SLIME_IDLE,
	SLIME_MOVING,
	SLIME_WIND_UP,
	SLIME_ATTACK,
	SLIME_RECOVER,

	// Tiles
	TILE_BLACK,
	TILE_LEFT_EDGE,
	TILE_RIGHT_EDGE,
	TILE_STRUCTURE,
	TILE_TOP_1,
	TILE_TOP_2,

	// Menus
	MAIN_MENU_BACKGROUND,
};

class AssetManager {
  public:
	[[nodiscard]] static AssetManager &getInstance();

	// Rule of Five: singleton is non-copyable and non-movable
	~AssetManager() = default;
	AssetManager(const AssetManager &) = delete;
	AssetManager &operator=(const AssetManager &) = delete;
	AssetManager(AssetManager &&) = delete;
	AssetManager &operator=(AssetManager &&) = delete;

	[[nodiscard]] const sf::Texture &getTexture(TextureAsset asset);

  private:
	AssetManager() = default;

	static std::string_view texturePath(TextureAsset asset);

	std::unordered_map<TextureAsset, sf::Texture> textures;
};
