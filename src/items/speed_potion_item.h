#pragma once
#include "item.h"

class SpeedPotionItem : public Item {
  public:
	SpeedPotionItem() = default;
	~SpeedPotionItem() = default;

	void activate(Player &player, Inventory &inventory, SlotRef ownSlot) override;

	[[nodiscard]] std::optional<SlotKind> equipmentSlot() const noexcept override { return std::nullopt; }
	[[nodiscard]] TextureAsset textureAsset() const noexcept override { return ITEM_SPEED_POTION; }
	[[nodiscard]] ItemInfo info() const override;
	json serialize() const override;
};
