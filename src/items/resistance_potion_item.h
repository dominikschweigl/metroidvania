#pragma once
#include "item.h"

class ResistancePotionItem : public Item {
  public:
	ResistancePotionItem() = default;
	~ResistancePotionItem() = default;

	bool activate(ActivateContext &ctx) override;

	[[nodiscard]] std::optional<SlotKind> equipmentSlot() const noexcept override { return std::nullopt; }
	[[nodiscard]] TextureAsset textureAsset() const noexcept override { return ITEM_RESISTANCE_POTION; }
	[[nodiscard]] ItemInfo info() const override;
	json serialize() const override;
};
