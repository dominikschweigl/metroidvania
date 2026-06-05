#pragma once
#include "item.h"

class UsbKeyItem : public Item {
  public:
	UsbKeyItem() = default;
	~UsbKeyItem() = default;

	void activate(Player &player, Inventory &inventory, SlotRef ownSlot) override;

	[[nodiscard]] std::optional<SlotKind> equipmentSlot() const noexcept override { return std::nullopt; }
	[[nodiscard]] TextureAsset textureAsset() const noexcept override { return ITEM_USB_KEY; }
	[[nodiscard]] ItemInfo info() const override;
	json serialize() const override;
};
