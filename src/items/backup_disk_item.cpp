#include "backup_disk_item.h"
#include "../entities/player/inventory.h"
#include "../entities/player/player.h"

bool BackupDiskItem::activate(ActivateContext &ctx)
{
	ctx.inventory.moveToEquipmentSlot(ctx.ownSlot);
	return true;
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
