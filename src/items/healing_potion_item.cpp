#include "healing_potion_item.h"
#include "../entities/player/inventory.h"
#include "../entities/player/player.h"
#include <string>

bool HealingPotionItem::activate(ActivateContext &ctx)
{
	ctx.player.heal(HEAL_AMOUNT);
	ctx.inventory.clearSlot(ctx.ownSlot);
	return true;
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
