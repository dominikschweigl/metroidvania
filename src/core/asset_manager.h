#pragma once
#include <SFML/Graphics.hpp>
#include <span>
#include <string_view>
#include <unordered_map>

enum TextureAsset {
	// Player
	PLAYER_IDLE_LOWER_BODY,
	PLAYER_IDLE_UPPER_BODY,
	PLAYER_WALK_LOWER_BODY,
	PLAYER_WALK_UPPER_BODY,
	PLAYER_RUN_LOWER_BODY,
	PLAYER_RUN_UPPER_BODY,
	PLAYER_JUMP_LOWER_BODY,
	PLAYER_JUMP_UPPER_BODY,
	PLAYER_ATTACK_SWING_UPPER_BODY,
	PLAYER_ATTACK_OVERHEAD_UPPER_BODY,
	PLAYER_ATTACK_THROW_HAT,
	PLAYER_HAT_PROJECTILE,
	PLAYER_HEAD,
	PLAYER_HEAD_HAT,
	PLAYER_WALL_SLIDE_LOWER_BODY,
	PLAYER_WALL_SLIDE_UPPER_BODY,

	// Race-condition slime
	SLIME_IDLE,
	SLIME_MOVING,
	SLIME_WIND_UP,
	SLIME_ATTACK,
	SLIME_RECOVER,

	// Capacitor
	CAPACITOR_HOVER,

	// Transistor Boss
	TRANSISTOR_BOSS_ROAMING,
	TRANSISTOR_BOSS_CHARGE_ATTACK_WINDUP,
	TRANSISTOR_BOSS_CHARGE_ATTACK,
	TRANSISTOR_BOSS_RECOVER,
	TRANSISTOR_BOSS_DEATH,

	// Tiles
	TILE_BLACK,
	TILE_LEFT_EDGE,
	TILE_RIGHT_EDGE,
	TILE_STRUCTURE,
	TILE_TOP_1,
	TILE_TOP_2,

	// Menus
	MAIN_MENU_BACKGROUND,

	// Items
	ITEM_HAT,
	ITEM_CHEWING_GUM,
	ITEM_HEALING_POTION,
	ITEM_JUMP_POTION,
	ITEM_SPEED_POTION,
	ITEM_RESISTANCE_POTION,
	ITEM_DAMAGE_POTION,
	ITEM_USB_KEY,
	ITEM_BACKUP_DISK,
};

enum FontAsset {
	UI_FONT,
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
	[[nodiscard]] const sf::Font &getFont(FontAsset asset);

  private:
	AssetManager() = default;

	static std::string_view texturePath(TextureAsset asset);
	static std::span<const char *const> fontCandidates(FontAsset asset);

	std::unordered_map<TextureAsset, sf::Texture> textures;
	std::unordered_map<FontAsset, sf::Font> fonts;
};
