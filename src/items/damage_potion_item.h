#pragma once
#include "item.h"

class DamagePotionItem : public Item {
  public:
	DamagePotionItem() = default;
	~DamagePotionItem() = default;

	bool activate(ActivateContext &ctx) override;

	[[nodiscard]] std::optional<SlotKind> equipmentSlot() const noexcept override { return std::nullopt; }
	[[nodiscard]] TextureAsset textureAsset() const noexcept override { return ITEM_DAMAGE_POTION; }
	[[nodiscard]] ItemInfo info() const override;
	json serialize() const override;
};
