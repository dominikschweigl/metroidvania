#include "chewing_gum_item.h"
#include "../entities/player/inventory.h"

void ChewingGumItem::activate(Player & /*player*/, Inventory &inventory, const SlotRef ownSlot)
{
	inventory.moveToEquipmentSlot(ownSlot);
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
