#pragma once
#include "item.h"

class BackupDiskItem : public Item {
  public:
	BackupDiskItem() = default;
	~BackupDiskItem() = default;

	void activate(Player &player, Inventory &inventory, SlotRef ownSlot) override;

	[[nodiscard]] std::optional<SlotKind> equipmentSlot() const noexcept override { return SlotKind::Backup; }
	[[nodiscard]] TextureAsset textureAsset() const noexcept override { return ITEM_BACKUP_DISK; }
	[[nodiscard]] ItemInfo info() const override;
};
