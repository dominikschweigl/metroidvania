#pragma once
#include "item.h"

class JumpPotionItem : public Item {
  public:
	JumpPotionItem() = default;
	~JumpPotionItem() = default;

	bool activate(ActivateContext &ctx) override;

	[[nodiscard]] std::optional<SlotKind> equipmentSlot() const noexcept override { return std::nullopt; }
	[[nodiscard]] TextureAsset textureAsset() const noexcept override { return ITEM_JUMP_POTION; }
	[[nodiscard]] ItemInfo info() const override;
	json serialize() const override;
};
