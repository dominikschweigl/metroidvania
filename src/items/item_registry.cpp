#include "item_registry.h"
#include "chewing_gum_item.h"
#include "hat_item.h"
#include "healing_potion_item.h"

std::span<const std::reference_wrapper<const Item>> registeredItems()
{
	static const HatItem hat;
	static const ChewingGumItem gum;
	static const HealingPotionItem potion;
	static const std::reference_wrapper<const Item> items[] = {hat, gum, potion};
	return items;
}
