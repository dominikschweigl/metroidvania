#include "asset_manager.h"
#include <format>
#include <stdexcept>
#include <string_view>

AssetManager &AssetManager::getInstance()
{
	static AssetManager instance;
	return instance;
}

const sf::Texture &AssetManager::getTexture(const TextureAsset asset)
{
	const auto [it, isInserted] = textures.try_emplace(asset);
	if (isInserted && !it->second.loadFromFile(std::string(texturePath(asset)))) {
		textures.erase(it);
		throw std::runtime_error(std::format("AssetManager: failed to load: {}", texturePath(asset)));
	}
	return it->second;
}

std::string_view AssetManager::texturePath(const TextureAsset asset)
{
	switch (asset) {
	case PLAYER_IDLE_HAT:
		return "./assets/images/player/idle_hat.png";
	case PLAYER_IDLE_LOWER_BODY:
		return "./assets/images/player/idle_lower_body_extended.png";
	case PLAYER_WALK_HAT:
		return "./assets/images/player/walk.png";
	case PLAYER_WALK_LOWER_BODY:
		return "./assets/images/player/walk_lower_body_extended.png";
	case PLAYER_WALK_UPPER_BODY:
		return "./assets/images/player/walk_upper_body.png";
	case PLAYER_RUN_HAT:
		return "./assets/images/player/run.png";
	case PLAYER_RUN_LOWER_BODY:
		return "./assets/images/player/run_lower_body_extended.png";
	case PLAYER_RUN_UPPER_BODY:
		return "./assets/images/player/run_upper_body.png";
	case PLAYER_JUMP:
		return "./assets/images/player/jump.png";
	case PLAYER_ATTACK_SWING_UPPER_BODY:
		return "./assets/images/player/attack_swing_upper_body_extended.png";
	case PLAYER_ATTACK_OVERHEAD_UPPER_BODY:
		return "./assets/images/player/attack_overhead_upper_body_extended.png";
	case SLIME_IDLE:
		return "./assets/images/enemies/race_condition/idle.png";
	case SLIME_MOVING:
		return "./assets/images/enemies/race_condition/moving.png";
	case SLIME_WIND_UP:
		return "./assets/images/enemies/race_condition/windup.png";
	case SLIME_ATTACK:
		return "./assets/images/enemies/race_condition/attack.png";
	case SLIME_RECOVER:
		return "./assets/images/enemies/race_condition/recover.png";
	case TILE_BLACK:
		return "assets/images/tiles/black.png";
	case TILE_LEFT_EDGE:
		return "assets/images/tiles/left_edge.png";
	case TILE_RIGHT_EDGE:
		return "assets/images/tiles/right_edge.png";
	case TILE_STRUCTURE:
		return "assets/images/tiles/structure.png";
	case TILE_TOP_1:
		return "assets/images/tiles/top1.png";
	case TILE_TOP_2:
		return "assets/images/tiles/top2.png";
	case MAIN_MENU_BACKGROUND:
		return "assets/images/menus/main_menu_background.jpeg";
	}

	throw std::logic_error("texturePath: missing TextureAsset path entry");
}
