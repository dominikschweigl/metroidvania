#include "backup_disk_item.h"
#include "../entities/player/inventory.h"
#include "../entities/player/player.h"

void BackupDiskItem::activate(Player & /*player*/, Inventory &inventory, const SlotRef ownSlot)
{
	inventory.moveToEquipmentSlot(ownSlot);
}

ItemInfo BackupDiskItem::info() const
{
	return {"Backup Disk", "An old floppy. Feels lucky.", "Auto-revives on death"};
}

json BackupDiskItem::serialize() const
{
	json j = Item::serialize();

	j["type"] = "BackupDiskItem";

	return j;
}
