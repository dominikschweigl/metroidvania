#pragma once
#include "item.h"

class HatItem : public Item {
  public:
	HatItem() = default;
	~HatItem() = default;

	void activate(Player &player, Inventory &inventory, SlotRef ownSlot) override;

	[[nodiscard]] std::optional<SlotKind> equipmentSlot() const noexcept override { return SlotKind::Hat; }
	[[nodiscard]] TextureAsset textureAsset() const noexcept override { return ITEM_HAT; }

	[[nodiscard]] ItemInfo info() const override;
};
