#include "jump_potion_item.h"
#include "../effects/effect.h"
#include "../entities/player/player.h"
#include <cmath>
#include <string>

void JumpPotionItem::activate(Player &player, Inventory &inventory, const SlotRef ownSlot)
{
	player.addEffect(Effect::jumpBoost());
	inventory.clearSlot(ownSlot);
}

ItemInfo JumpPotionItem::info() const
{
	const Effect effect = Effect::jumpBoost();
	const int jumpPct = static_cast<int>(std::roundf((effect.jumpMultiplier() - 1.f) * 100.f));
	const int seconds = static_cast<int>(effect.totalDuration);
	return {"Jump Boost Potion", "A fizzy brew that defies gravity.",
	        "+" + std::to_string(jumpPct) + "% jump for " + std::to_string(seconds) + " s"};
}
