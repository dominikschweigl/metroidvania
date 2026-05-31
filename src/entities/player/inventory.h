#pragma once
#include "../../items/item.h"
#include <array>
#include <memory>
#include <span>

class Player;

class Inventory {
  public:
	static constexpr int EQUIPMENT_SIZE = 2;
	static constexpr int GRID_SIZE = 24;
	static constexpr int HOTBAR_SIZE = 5;

	Inventory() = default;
	~Inventory() = default;

	Inventory(const Inventory &) = delete;
	Inventory &operator=(const Inventory &) = delete;
	Inventory(Inventory &&) = delete;
	Inventory &operator=(Inventory &&) = delete;

	std::unique_ptr<Item> hatSlot;
	std::unique_ptr<Item> gumSlot;
	std::array<std::unique_ptr<Item>, GRID_SIZE> grid;
	std::array<std::unique_ptr<Item>, HOTBAR_SIZE> hotbar;

	void addItem(std::unique_ptr<Item> item);

	[[nodiscard]] static std::span<const SlotRef> slots() noexcept { return ALL_SLOTS_; }
	[[nodiscard]] static std::span<const SlotRef> equipmentSlots() noexcept { return EQUIPMENT_SLOTS_; }
	[[nodiscard]] static std::span<const SlotRef> gridSlots() noexcept { return GRID_SLOTS_; }
	[[nodiscard]] static std::span<const SlotRef> hotbarSlots() noexcept { return HOTBAR_SLOTS_; }

	[[nodiscard]] bool hasHat() const noexcept { return hatSlot != nullptr; }
	[[nodiscard]] bool hasGum() const noexcept { return gumSlot != nullptr; }

	[[nodiscard]] bool hasItem(SlotRef slot) const noexcept;

	// Precondition: hasItem(slot) == true.
	[[nodiscard]] Item &itemAt(SlotRef slot);
	[[nodiscard]] const Item &itemAt(SlotRef slot) const;

	// Single entry point: calls item.activate(player, *this, slot).
	// Handles both temporary consumption and permanent re-equip transparently.
	void interact(SlotRef slot, Player &player);

	// Move item between any two compatible slots. Returns false if invalid.
	bool moveItem(SlotRef from, SlotRef to);

	// Called by item activate() implementations.
	void clearSlot(SlotRef slot);
	void moveToEquipmentSlot(SlotRef from);

  private:
	[[nodiscard]] std::unique_ptr<Item> &slotRef(SlotRef slot);
	[[nodiscard]] const std::unique_ptr<Item> &slotRef(SlotRef slot) const;

	static bool isValidInSlot(const Item &item, SlotKind slotKind) noexcept;

	static constexpr std::array<SlotRef, EQUIPMENT_SIZE> EQUIPMENT_SLOTS_ = {
	    SlotRef{SlotKind::Hat, 0},
	    SlotRef{SlotKind::Gum, 0},
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
