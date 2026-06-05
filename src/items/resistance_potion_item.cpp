#include "resistance_potion_item.h"
#include "../effects/effect.h"
#include "../entities/player/player.h"
#include <cmath>
#include <string>

void ResistancePotionItem::activate(Player &player, Inventory &inventory, const SlotRef ownSlot)
{
	player.addEffect(Effect::resistance());
	inventory.clearSlot(ownSlot);
}

ItemInfo ResistancePotionItem::info() const
{
	const Effect effect = Effect::resistance();
	const int resPct = static_cast<int>(std::roundf(effect.damageResistance() * 100.f));
	const int seconds = static_cast<int>(effect.totalDuration);
	return {"Resistance Potion", "A thick, bitter sip that hardens your resolve.",
	        std::to_string(resPct) + "% damage reduction for " + std::to_string(seconds) + " s"};
}

json ResistancePotionItem::serialize() const
{
	json j = Item::serialize();

	j["type"] = "ResistancePotionItem";

	return j;
}
