#include "asset_manager.h"
#include <format>
#include <span>
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

const sf::Font &AssetManager::getFont(const FontAsset asset)
{
	const auto [iterator, isInserted] = fonts.try_emplace(asset);
	if (isInserted) {
		bool loaded = false;
		for (const char *path : fontCandidates(asset)) {
			if (iterator->second.openFromFile(path)) {
				loaded = true;
				break;
			}
		}
		if (!loaded) {
			fonts.erase(iterator);
			throw std::runtime_error(
			    std::format("AssetManager: no usable font found for FontAsset {}", static_cast<int>(asset)));
		}
	}
	return iterator->second;
}

std::span<const char *const> AssetManager::fontCandidates(const FontAsset asset)
{
	switch (asset) {
	case UI_FONT: {
		static constexpr std::array<const char *, 7> candidates = {
		    "C:\\Windows\\Fonts\\arial.ttf",
		    "C:\\Windows\\Fonts\\segoeui.ttf",
		    "C:\\Windows\\Fonts\\calibri.ttf",
		    "/usr/share/fonts/liberation-sans-fonts/LiberationSans-Regular.ttf",
		    "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
		    "/usr/share/fonts/dejavu-sans-fonts/DejaVuSans.ttf",
		    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
		};
		return candidates;
	}
	}
	throw std::logic_error("fontCandidates: missing FontAsset entry");
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
	case RESISTOR_BUG_IDLE:
		return "./assets/images/enemies/resistor_bug/idle.png";
	case RESISTOR_BUG_MOVING:
		return "./assets/images/enemies/resistor_bug/moving.png";
	case CAPACITOR_HOVER:
		return "./assets/images/enemies/capacitor/capacitor_hover.png";
	case TRANSISTOR_BOSS_ROAMING:
		return "./assets/images/enemies/bosses/transistor_boss/roaming.png";
	case TRANSISTOR_BOSS_CHARGE_ATTACK_WINDUP:
		return "./assets/images/enemies/bosses/transistor_boss/charge_attack_windup.png";
	case TRANSISTOR_BOSS_CHARGE_ATTACK:
		return "./assets/images/enemies/bosses/transistor_boss/charge_attack.png";
	case TRANSISTOR_BOSS_RECOVER:
		return "./assets/images/enemies/bosses/transistor_boss/recover.png";
	case TRANSISTOR_BOSS_DEATH:
		return "./assets/images/enemies/bosses/transistor_boss/death_animation.png";
	case SEGFAULT_BOSS_IDLE:
		return "./assets/images/enemies/bosses/segfault_boss/idle.png";
	case SEGFAULT_BOSS_BEGIN_ROAMING:
		return "./assets/images/enemies/bosses/segfault_boss/begin_roaming.png";
	case SEGFAULT_BOSS_ROAMING:
		return "./assets/images/enemies/bosses/segfault_boss/roaming.png";
	case SEGFAULT_BOSS_CHARGE:
		return "./assets/images/enemies/bosses/segfault_boss/charge.png";
	case SEGFAULT_BOSS_DEATH:
		return "./assets/images/enemies/bosses/segfault_boss/death.png";
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
	case SEGFAULT_FRAME_0:
		return "assets/images/menus/segfault_frame_0.png";
	case SEGFAULT_FRAME_1:
		return "assets/images/menus/segfault_frame_1.png";
	case SEGFAULT_FRAME_2:
		return "assets/images/menus/segfault_frame_2.png";
	case ITEM_HAT:
		return "assets/images/items/hat.png";
	case ITEM_CHEWING_GUM:
		return "assets/images/items/chewing_gum.png";
	case ITEM_HEALING_POTION:
		return "assets/images/items/healing_potion.png";
	case ITEM_JUMP_POTION:
		return "assets/images/items/jump_boost_potion.png";
	case ITEM_SPEED_POTION:
		return "assets/images/items/speed_potion.png";
	case ITEM_RESISTANCE_POTION:
		return "assets/images/items/resistance_potion.png";
	case ITEM_DAMAGE_POTION:
		return "assets/images/items/damage_potion.png";
	case ITEM_USB_KEY:
		return "assets/images/items/keys/damaged_usb_key.png";
	case ITEM_BACKUP_DISK:
		return "assets/images/items/floppy_disk/floppy_disk_blackwhite.png";
	}

	throw std::logic_error("texturePath: missing TextureAsset path entry");
}
