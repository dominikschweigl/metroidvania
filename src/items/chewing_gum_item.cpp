#include "chewing_gum_item.h"
#include "../entities/player/inventory.h"

bool ChewingGumItem::activate(ActivateContext &ctx)
{
	ctx.inventory.moveToEquipmentSlot(ctx.ownSlot);
	return true;
}

ItemInfo ChewingGumItem::info() const
{
	return {"Sticky Chewing Gum", "Clings to any surface.", "Enables wall slide"};
}

json ChewingGumItem::serialize() const
{
	json j = Item::serialize();

	j["type"] = "ChewingGumItem";

	return j;
}
