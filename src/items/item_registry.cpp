#include "item_registry.h"
#include "backup_disk_item.h"
#include "chewing_gum_item.h"
#include "damage_potion_item.h"
#include "hat_item.h"
#include "healing_potion_item.h"
#include "jump_potion_item.h"
#include "resistance_potion_item.h"
#include "speed_potion_item.h"
#include "usb_key_item.h"

std::span<const std::reference_wrapper<const Item>> registeredItems()
{
	static const HatItem hat;
	static const ChewingGumItem gum;
	static const HealingPotionItem healingPotion;
	static const JumpPotionItem jumpPotion;
	static const SpeedPotionItem speedPotion;
	static const ResistancePotionItem resistancePotion;
	static const DamagePotionItem damagePotion;
	static const UsbKeyItem usbKey;
	static const BackupDiskItem backupDisk;
	static const std::reference_wrapper<const Item> items[] = {
	    hat, gum, healingPotion, jumpPotion, speedPotion, resistancePotion, damagePotion, usbKey, backupDisk};
	return items;
}
