#pragma once
#include "../world/Room.hpp"
#include "item.h"

class UsbKeyItem : public Item {
  public:
	UsbKeyItem() = default;
	~UsbKeyItem() = default;

	bool activate(ActivateContext &ctx) override;

	[[nodiscard]] std::optional<SlotKind> equipmentSlot() const noexcept override { return std::nullopt; }
	[[nodiscard]] TextureAsset textureAsset() const noexcept override { return ITEM_USB_KEY; }
	[[nodiscard]] ItemInfo info() const override;
	json serialize() const override;
};
