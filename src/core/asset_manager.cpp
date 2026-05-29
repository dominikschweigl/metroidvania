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
	const auto [iterator, isInserted] = textures.try_emplace(asset);
	if (isInserted && !iterator->second.loadFromFile(std::string(texturePath(asset)))) {
		textures.erase(iterator);
		throw std::runtime_error(std::format("AssetManager: failed to load: {}", texturePath(asset)));
	}
	iterator->second.setSmooth(false);
	return iterator->second;
}

std::string_view AssetManager::texturePath(const TextureAsset asset)
{
	switch (asset) {
	case PLAYER_IDLE_LOWER_BODY:
		return "./assets/images/player/idle_lower_body_extended.png";
	case PLAYER_IDLE_UPPER_BODY:
		return "./assets/images/player/idle_upper_body_extended_no_head.png";
	case PLAYER_WALK_LOWER_BODY:
		return "./assets/images/player/walk_lower_body_extended.png";
	case PLAYER_WALK_UPPER_BODY:
		return "./assets/images/player/walk_upper_body_extended_no_head.png";
	case PLAYER_RUN_LOWER_BODY:
		return "./assets/images/player/run_lower_body_extended.png";
	case PLAYER_RUN_UPPER_BODY:
		return "./assets/images/player/run_upper_body_extended_no_head.png";
	case PLAYER_JUMP_LOWER_BODY:
		return "./assets/images/player/jump_lower_body_extended.png";
	case PLAYER_JUMP_UPPER_BODY:
		return "./assets/images/player/jump_upper_body_extended_no_head.png";
	case PLAYER_ATTACK_SWING_UPPER_BODY:
		return "./assets/images/player/attack_swing_upper_body_extended_no_head.png";
	case PLAYER_ATTACK_OVERHEAD_UPPER_BODY:
		return "./assets/images/player/attack_overhead_upper_body_extended_no_head.png";
	case PLAYER_ATTACK_THROW_HAT:
		return "./assets/images/player/attack_throw_hat_upper_body_no_head.png";
	case PLAYER_HAT_PROJECTILE:
		return "./assets/images/player/hat_projectile.png";
	case PLAYER_HEAD:
		return "./assets/images/player/head.png";
	case PLAYER_HEAD_HAT:
		return "./assets/images/player/head_hat.png";
	case PLAYER_WALL_SLIDE_LOWER_BODY:
		return "./assets/images/player/wall_slide_lower_body_extended.png";
	case PLAYER_WALL_SLIDE_UPPER_BODY:
		return "./assets/images/player/wall_slide_upper_body_extended_no_head.png";
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
