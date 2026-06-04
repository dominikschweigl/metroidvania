#include "hat_item.h"
#include "../entities/player/inventory.h"

void HatItem::activate(Player & /*player*/, Inventory &inventory, const SlotRef ownSlot)
{
	inventory.moveToEquipmentSlot(ownSlot);
}

ItemInfo HatItem::info() const
{
	return {"The Debugger's Hat", "A sentient hat that loves being thrown.", "Enables hat throw"};
}
