#pragma once
#include "../items/backup_disk_item.h"
#include "../items/chewing_gum_item.h"
#include "../items/damage_potion_item.h"
#include "../items/hat_item.h"
#include "../items/healing_potion_item.h"
#include "../items/item.h"
#include "../items/jump_potion_item.h"
#include "../items/resistance_potion_item.h"
#include "../items/speed_potion_item.h"
#include "../items/usb_key_item.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct ItemFactory {
	static std::unique_ptr<Item> create(const json &j)
	{
		if (j.empty())
			return nullptr;
		std::string type = j["type"];

		if (type == "ChewingGumItem") {
			return std::make_unique<ChewingGumItem>();
		}

		if (type == "HatItem") {
			return std::make_unique<HatItem>();
		}

		if (type == "HealingPotionItem") {
			return std::make_unique<HealingPotionItem>();
		}

		if (type == "JumpPotionItem") {
			return std::make_unique<JumpPotionItem>();
		}

		if (type == "ResistancePotionItem") {
			return std::make_unique<ResistancePotionItem>();
		}

		if (type == "SpeedPotionItem") {
			return std::make_unique<SpeedPotionItem>();
		}

		if (type == "DamagePotionItem") {
			return std::make_unique<DamagePotionItem>();
		}

		if (type == "UsbKeyItem") {
			return std::make_unique<UsbKeyItem>();
		}

		if (type == "BackupDiskItem") {
			return std::make_unique<BackupDiskItem>();
		}

		return nullptr;
	}
};
