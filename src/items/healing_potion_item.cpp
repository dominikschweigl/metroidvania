#include "healing_potion_item.h"
#include "../entities/player/inventory.h"
#include "../entities/player/player.h"
#include <string>

void HealingPotionItem::activate(Player &player, Inventory &inventory, const SlotRef ownSlot)
{
	player.heal(HEAL_AMOUNT);
	inventory.clearSlot(ownSlot);
}

ItemInfo HealingPotionItem::info() const
{
	return {"Health Potion", "A warm, glowing liquid.", "Restores " + std::to_string(HEAL_AMOUNT) + " HP"};
}

json HealingPotionItem::serialize() const
{
	json j = Item::serialize();

	j["type"] = "HealingPotionItem";

	return j;
}
