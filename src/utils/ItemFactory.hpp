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
#include <functional>
#include <iostream>
#include <nlohmann/json.hpp>
#include <unordered_map>

using json = nlohmann::json;

struct ItemFactory {
	static std::unique_ptr<Item> create(const json &j)
	{
		if (j.empty())
			return nullptr;

		using Creator = std::function<std::unique_ptr<Item>()>;
		static const std::unordered_map<std::string, Creator> table = {
		    {"ChewingGumItem", [] { return std::make_unique<ChewingGumItem>(); }},
		    {"HatItem", [] { return std::make_unique<HatItem>(); }},
		    {"HealingPotionItem", [] { return std::make_unique<HealingPotionItem>(); }},
		    {"JumpPotionItem", [] { return std::make_unique<JumpPotionItem>(); }},
		    {"ResistancePotionItem", [] { return std::make_unique<ResistancePotionItem>(); }},
		    {"SpeedPotionItem", [] { return std::make_unique<SpeedPotionItem>(); }},
		    {"DamagePotionItem", [] { return std::make_unique<DamagePotionItem>(); }},
		    {"UsbKeyItem", [] { return std::make_unique<UsbKeyItem>(); }},
		    {"BackupDiskItem", [] { return std::make_unique<BackupDiskItem>(); }},
		};

		const std::string type = j.value("type", "");
		auto it = table.find(type);
		if (it == table.end()) {
			std::cerr << "ItemFactory: unknown type '" << type << "'\n";
			return nullptr;
		}

		return it->second();
	}
};
