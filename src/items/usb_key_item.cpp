#include "usb_key_item.h"
#include "../entities/player/inventory.h"
#include "../entities/player/player.h"
#include "../world/world.h"

bool UsbKeyItem::activate(ActivateContext &ctx)
{
	if (!ctx.world)
		return false;

	Door *door = ctx.world->getTouchingDoor(ctx.player.getBounds());
	if (!door || !door->locked)
		return false;

	door->locked = false;
	ctx.inventory.clearSlot(ctx.ownSlot);
	return true;
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
