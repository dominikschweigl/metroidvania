#include "hat_item.h"
#include "../entities/player/inventory.h"

bool HatItem::activate(ActivateContext &ctx)
{
	ctx.inventory.moveToEquipmentSlot(ctx.ownSlot);
	return true;
}

ItemInfo HatItem::info() const
{
	return {"The Debugger's Hat", "A sentient hat that loves being thrown.", "Enables hat throw"};
}

json HatItem::serialize() const
{
	json j = Item::serialize();

	j["type"] = "HatItem";

	return j;
}
