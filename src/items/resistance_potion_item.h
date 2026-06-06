#pragma once
#include "item.h"

class ResistancePotionItem : public Item {
  public:
	ResistancePotionItem() = default;
	~ResistancePotionItem() = default;

	void activate(Player &player, Inventory &inventory, SlotRef ownSlot) override;

	[[nodiscard]] std::optional<SlotKind> equipmentSlot() const noexcept override { return std::nullopt; }
	[[nodiscard]] TextureAsset textureAsset() const noexcept override { return ITEM_RESISTANCE_POTION; }
	[[nodiscard]] ItemInfo info() const override;
	json serialize() const override;
};
