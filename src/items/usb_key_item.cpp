#include "usb_key_item.h"

void UsbKeyItem::activate(Player & /*player*/, Inventory & /*inventory*/, const SlotRef /*ownSlot*/) {}

ItemInfo UsbKeyItem::info() const
{
	return {"Damaged USB Key", "A worn drive. Maybe it opens something?", "???"};
}
