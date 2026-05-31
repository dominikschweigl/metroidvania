#pragma once
#include "item.h"

class ChewingGumItem : public Item {
  public:
	ChewingGumItem()  = default;
	~ChewingGumItem() = default;

	void activate(Player &player, Inventory &inventory, SlotRef ownSlot) override;

	[[nodiscard]] std::optional<SlotKind> equipmentSlot() const noexcept override { return SlotKind::Gum; }
	[[nodiscard]] TextureAsset            textureAsset()  const noexcept override { return ITEM_CHEWING_GUM; }

	[[nodiscard]] ItemInfo info() const noexcept override
	{
		return {"Sticky Chewing Gum", "Clings to any surface.", "Enables wall slide"};
	}
};
