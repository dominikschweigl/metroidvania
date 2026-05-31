#include "hat_item.h"
#include "../entities/player/inventory.h"

void HatItem::activate(Player & /*player*/, Inventory &inventory, const SlotRef ownSlot)
{
	inventory.moveToEquipmentSlot(ownSlot);
}
