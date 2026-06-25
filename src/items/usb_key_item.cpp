#include "usb_key_item.h"
#include "../entities/player/inventory.h"

void UsbKeyItem::activate(Player & /*player*/, Inventory &inventory, const SlotRef ownSlot)
{
	inventory.clearSlot(ownSlot);
}

ItemInfo UsbKeyItem::info() const
{
	return {"Damaged USB Key", "A worn drive. Maybe it opens something?", "???"};
}

json UsbKeyItem::serialize() const
{
	json j = Item::serialize();

	j["type"] = "UsbKeyItem";

	return j;
}
