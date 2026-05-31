#include "chewing_gum_item.h"
#include "../entities/player/inventory.h"

void ChewingGumItem::activate(Player & /*player*/, Inventory &inventory, const SlotRef ownSlot)
{
	inventory.moveToEquipmentSlot(ownSlot);
}
