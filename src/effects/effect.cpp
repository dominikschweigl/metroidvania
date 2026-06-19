#include "effect.h"

Effect::Effect(float totalDuration, TextureAsset icon, std::string_view name, std::string_view effectId,
               float speedMult, float jumpMult, float damageMult, float damageRes) noexcept
    : remainingDuration(totalDuration), totalDuration(totalDuration), icon_(icon), name_(name), effectId_(effectId),
      speedMult_(speedMult), jumpMult_(jumpMult), damageMult_(damageMult), damageRes_(damageRes)
{
}

Effect Effect::jumpBoost() noexcept
{
	return Effect{30.f, ITEM_JUMP_POTION, "Jump Boost", "jump_boost", 1.f, 1.4f, 1.f, 0.f};
}

Effect Effect::speed() noexcept
{
	return Effect{30.f, ITEM_SPEED_POTION, "Speed Boost", "speed_boost", 1.5f, 1.f, 1.f, 0.f};
}

Effect Effect::damage() noexcept
{
	return Effect{30.f, ITEM_DAMAGE_POTION, "Damage Boost", "damage_boost", 1.f, 1.f, 2.f, 0.f};
}

Effect Effect::resistance() noexcept
{
	return Effect{30.f, ITEM_RESISTANCE_POTION, "Resistance", "resistance", 1.f, 1.f, 1.f, 0.5f};
}

Effect Effect::slow() noexcept
{
	return Effect{4.f, ITEM_CHEWING_GUM, "Slow", "slow", 0.5f, 1.f, 1.f, 0.f};
}
