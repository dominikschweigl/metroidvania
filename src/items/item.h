#pragma once
#include "../core/asset_manager.h"
#include "slot_ref.h"
#include <memory>
#include <optional>
#include <string_view>

class Player;
class Inventory;

struct ItemInfo {
	std::string_view name;
	std::string_view description;
	std::string_view effect;
};

class Item {
  public:
	virtual ~Item() = default;

	virtual void activate(Player &player, Inventory &inventory, SlotRef ownSlot) = 0;

	[[nodiscard]] virtual std::optional<SlotKind> equipmentSlot() const noexcept = 0;
	[[nodiscard]] virtual ItemInfo info() const noexcept = 0;
	[[nodiscard]] virtual TextureAsset textureAsset() const noexcept = 0;

	Item(const Item &) = delete;
	Item &operator=(const Item &) = delete;
	Item(Item &&) = delete;
	Item &operator=(Item &&) = delete;

  protected:
	Item() = default;
};
