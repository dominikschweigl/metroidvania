#include "speed_potion_item.h"
#include "../effects/effect.h"
#include "../entities/player/player.h"
#include <cmath>
#include <string>

bool SpeedPotionItem::activate(ActivateContext &ctx)
{
	ctx.player.addEffect(Effect::speed());
	ctx.inventory.clearSlot(ctx.ownSlot);
	return true;
}

ItemInfo SpeedPotionItem::info() const
{
	const Effect effect = Effect::speed();
	const int speedPct = static_cast<int>(std::roundf((effect.speedMultiplier() - 1.f) * 100.f));
	const int seconds = static_cast<int>(effect.totalDuration);
	return {"Speed Potion", "A crackling liquid that buzzes under your skin.",
	        "+" + std::to_string(speedPct) + "% speed for " + std::to_string(seconds) + " s"};
}

json SpeedPotionItem::serialize() const
{
	json j = Item::serialize();

	j["type"] = "SpeedPotionItem";

	return j;
}
