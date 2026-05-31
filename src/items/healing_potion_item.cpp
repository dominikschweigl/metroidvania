#include "healing_potion_item.h"
#include "../entities/player/inventory.h"
#include "../entities/player/player.h"

void HealingPotionItem::activate(Player &player, Inventory &inventory, const SlotRef ownSlot)
{
	player.heal(2);
	inventory.clearSlot(ownSlot);
}
