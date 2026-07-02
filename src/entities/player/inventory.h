#pragma once
#include "../../items/item.h"
#include <array>
#include <memory>
#include <span>

class Player;

class Inventory {
  public:
	static constexpr int EQUIPMENT_SIZE = 3;
	static constexpr int GRID_SIZE = 24;
	static constexpr int HOTBAR_SIZE = 7;

	Inventory() = default;
	~Inventory() = default;

	Inventory(const Inventory &) = delete;
	Inventory &operator=(const Inventory &) = delete;
	Inventory(Inventory &&) = delete;
	Inventory &operator=(Inventory &&) = delete;

	std::unique_ptr<Item> hatSlot;
	std::unique_ptr<Item> gumSlot;
	std::unique_ptr<Item> backupSlot;
	std::array<std::unique_ptr<Item>, GRID_SIZE> grid;
	std::array<std::unique_ptr<Item>, HOTBAR_SIZE> hotbar;

	void addItem(std::unique_ptr<Item> item);
	[[nodiscard]] bool canAdd(const Item &item) const noexcept;

	[[nodiscard]] static std::span<const SlotRef> slots() noexcept { return ALL_SLOTS_; }
	[[nodiscard]] static std::span<const SlotRef> equipmentSlots() noexcept { return EQUIPMENT_SLOTS_; }
	[[nodiscard]] static std::span<const SlotRef> gridSlots() noexcept { return GRID_SLOTS_; }
	[[nodiscard]] static std::span<const SlotRef> hotbarSlots() noexcept { return HOTBAR_SLOTS_; }

	[[nodiscard]] bool hasHat() const noexcept { return hatSlot != nullptr; }
	[[nodiscard]] bool hasGum() const noexcept { return gumSlot != nullptr; }
	[[nodiscard]] bool hasBackup() const noexcept { return backupSlot != nullptr; }

	[[nodiscard]] bool hasItem(const SlotRef slot) const noexcept;

	[[nodiscard]] Item &itemAt(const SlotRef slot);
	[[nodiscard]] const Item &itemAt(const SlotRef slot) const;

	void interact(const SlotRef slot, Player &player, World *world);

	// Move item between any two compatible slots. Returns false if invalid.
	bool moveItem(const SlotRef from, const SlotRef to);

	void clearSlot(const SlotRef slot);
	void moveToEquipmentSlot(const SlotRef from);

	// Flattens the inventory into a vector
	std::vector<Item *> flatten() const;

	json serialize() const;
	void deserialize(const json &j);

  private:
	[[nodiscard]] std::unique_ptr<Item> &slotRef(const SlotRef slot);
	[[nodiscard]] const std::unique_ptr<Item> &slotRef(const SlotRef slot) const;

	static bool isValidInSlot(const Item &item, SlotKind slotKind) noexcept;
	static bool isEquipmentSlot(SlotKind slotKind) noexcept;

	static constexpr std::array<SlotRef, EQUIPMENT_SIZE> EQUIPMENT_SLOTS_ = {
	    SlotRef{SlotKind::Hat, 0},
	    SlotRef{SlotKind::Gum, 0},
	    SlotRef{SlotKind::Backup, 0},
	};
	static constexpr std::array<SlotRef, GRID_SIZE> GRID_SLOTS_ = [] {
		std::array<SlotRef, GRID_SIZE> slots{};
		for (int i = 0; i < GRID_SIZE; ++i)
			slots[i] = {SlotKind::Grid, i};
		return slots;
	}();
	static constexpr std::array<SlotRef, HOTBAR_SIZE> HOTBAR_SLOTS_ = [] {
		std::array<SlotRef, HOTBAR_SIZE> slots{};
		for (int i = 0; i < HOTBAR_SIZE; ++i)
			slots[i] = {SlotKind::Hotbar, i};
		return slots;
	}();
	static constexpr std::array<SlotRef, EQUIPMENT_SIZE + GRID_SIZE + HOTBAR_SIZE> ALL_SLOTS_ = [] {
		std::array<SlotRef, EQUIPMENT_SIZE + GRID_SIZE + HOTBAR_SIZE> slots{};
		std::size_t idx = 0;
		for (const SlotRef &s : EQUIPMENT_SLOTS_)
			slots[idx++] = s;
		for (const SlotRef &s : GRID_SLOTS_)
			slots[idx++] = s;
		for (const SlotRef &s : HOTBAR_SLOTS_)
			slots[idx++] = s;
		return slots;
	}();
};
