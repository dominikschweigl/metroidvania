#pragma once
#include "item.h"

class HatItem : public Item {
  public:
	HatItem() = default;
	~HatItem() = default;

	bool activate(ActivateContext &ctx) override;

	[[nodiscard]] std::optional<SlotKind> equipmentSlot() const noexcept override { return SlotKind::Hat; }
	[[nodiscard]] TextureAsset textureAsset() const noexcept override { return ITEM_HAT; }

	[[nodiscard]] ItemInfo info() const override;
	json serialize() const override;
};
