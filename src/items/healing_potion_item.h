#pragma once
#include "item.h"

class HealingPotionItem : public Item {
  public:
	HealingPotionItem()  = default;
	~HealingPotionItem() = default;

	void activate(Player &player, Inventory &inventory, SlotRef ownSlot) override;

	[[nodiscard]] std::optional<SlotKind> equipmentSlot() const noexcept override { return std::nullopt; }
	[[nodiscard]] TextureAsset            textureAsset()  const noexcept override { return ITEM_HEALING_POTION; }

	[[nodiscard]] ItemInfo info() const noexcept override
	{
		return {"Health Potion", "A warm, glowing liquid.", "Restores 2 HP"};
	}
};
