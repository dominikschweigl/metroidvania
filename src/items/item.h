#pragma once
#include "../core/asset_manager.h"
#include "slot_ref.h"
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <string_view>

using json = nlohmann::json;

class Player;
class Inventory;
class World;

struct ActivateContext {
	Player &player;
	Inventory &inventory;
	SlotRef ownSlot;
	World *world;
};

struct ItemInfo {
	std::string_view name;
	std::string_view description;
	std::string effect;
};

class Item {
  public:
	virtual ~Item() = default;

	virtual bool activate(ActivateContext &ctx) = 0;

	[[nodiscard]] virtual std::optional<SlotKind> equipmentSlot() const noexcept = 0;
	[[nodiscard]] virtual ItemInfo info() const = 0;
	[[nodiscard]] virtual TextureAsset textureAsset() const noexcept = 0;
	virtual json serialize() const { return json{}; }
	virtual void serialize(const json &j) const {};

	Item(const Item &) = delete;
	Item &operator=(const Item &) = delete;
	Item(Item &&) = default;
	Item &operator=(Item &&) = default;

  protected:
	Item() = default;
};
