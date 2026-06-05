#include "damage_potion_item.h"
#include "../effects/effect.h"
#include "../entities/player/player.h"
#include <cmath>
#include <string>

void DamagePotionItem::activate(Player &player, Inventory &inventory, const SlotRef ownSlot)
{
	player.addEffect(Effect::damage());
	inventory.clearSlot(ownSlot);
}

ItemInfo DamagePotionItem::info() const
{
	const Effect effect = Effect::damage();
	const int damagePct = static_cast<int>(std::roundf((effect.damageMultiplier() - 1.f) * 100.f));
	const int seconds = static_cast<int>(effect.totalDuration);
	return {"Damage Potion", "Smells like rage. Tastes like it too.",
	        "+" + std::to_string(damagePct) + "% damage for " + std::to_string(seconds) + " s"};
}

json DamagePotionItem::serialize() const
{
	json j = Item::serialize();

	j["type"] = "DamagePotionItem";

	return j;
}
