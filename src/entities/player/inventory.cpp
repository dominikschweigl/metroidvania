#include "inventory.h"
#include "../../utils/ItemFactory.hpp"
#include "player.h"
#include <stdexcept>

bool Inventory::isEquipmentSlot(const SlotKind slotkind) noexcept
{
	for (const SlotRef &s : EQUIPMENT_SLOTS_) {
		if (s.kind == slotkind) {
			return true;
		}
	}
	return false;
}

bool Inventory::isValidInSlot(const Item &item, const SlotKind slotKind) noexcept
{
	if (!isEquipmentSlot(slotKind)) {
		return true;
	}
	const std::optional<SlotKind> equipment = item.equipmentSlot();
	return equipment.has_value() && *equipment == slotKind;
}

std::unique_ptr<Item> &Inventory::slotRef(const SlotRef slot)
{
	switch (slot.kind) {
	case SlotKind::Hat:
		return hatSlot;
	case SlotKind::Gum:
		return gumSlot;
	case SlotKind::Backup:
		return backupSlot;
	case SlotKind::Grid:
		return grid[slot.index];
	case SlotKind::Hotbar:
		return hotbar[slot.index];
	}
	throw std::logic_error("Inventory::slotRef: invalid SlotKind");
}

const std::unique_ptr<Item> &Inventory::slotRef(const SlotRef slot) const
{
	switch (slot.kind) {
	case SlotKind::Hat:
		return hatSlot;
	case SlotKind::Gum:
		return gumSlot;
	case SlotKind::Backup:
		return backupSlot;
	case SlotKind::Grid:
		return grid[slot.index];
	case SlotKind::Hotbar:
		return hotbar[slot.index];
	}
	throw std::logic_error("Inventory::slotRef: invalid SlotKind");
}

bool Inventory::hasItem(const SlotRef slot) const noexcept
{
	return slotRef(slot) != nullptr;
}

Item &Inventory::itemAt(const SlotRef slot)
{
	return *slotRef(slot);
}

const Item &Inventory::itemAt(const SlotRef slot) const
{
	return *slotRef(slot);
}

bool Inventory::canAdd(const Item &item) const noexcept
{
	const std::optional<SlotKind> dest = item.equipmentSlot();
	if (dest && !slotRef({*dest, 0}))
		return true;
	for (const std::unique_ptr<Item> &cell : grid)
		if (!cell)
			return true;
	for (const std::unique_ptr<Item> &cell : hotbar)
		if (!cell)
			return true;
	return false;
}

void Inventory::addItem(std::unique_ptr<Item> item)
{
	const std::optional<SlotKind> dest = item->equipmentSlot();
	if (dest) {
		std::unique_ptr<Item> &slot = slotRef({*dest, 0});
		if (!slot) {
			slot = std::move(item);
			return;
		}
	}
	for (std::unique_ptr<Item> &cell : grid) {
		if (!cell) {
			cell = std::move(item);
			return;
		}
	}
	for (std::unique_ptr<Item> &cell : hotbar) {
		if (!cell) {
			cell = std::move(item);
			return;
		}
	}
}

void Inventory::interact(const SlotRef slot, Player &player)
{
	if (hasItem(slot))
		itemAt(slot).activate(player, *this, slot);
}

bool Inventory::moveItem(const SlotRef from, const SlotRef to)
{
	if (from.kind == to.kind && from.index == to.index)
		return false;
	std::unique_ptr<Item> &src = slotRef(from);
	if (!src)
		return false;
	if (!isValidInSlot(*src, to.kind))
		return false;
	std::unique_ptr<Item> &dst = slotRef(to);
	if (dst && !isValidInSlot(*dst, from.kind))
		return false;
	std::swap(src, dst);
	return true;
}

void Inventory::clearSlot(const SlotRef slot)
{
	slotRef(slot).reset();
}

void Inventory::moveToEquipmentSlot(const SlotRef from)
{
	std::unique_ptr<Item> &srcItem = slotRef(from);
	if (!srcItem)
		return;
	const std::optional<SlotKind> equipmentSlot = srcItem->equipmentSlot();
	if (!equipmentSlot || from.kind == *equipmentSlot)
		return;
	std::swap(slotRef({*equipmentSlot, 0}), srcItem);
}

std::vector<Item *> Inventory::flatten() const
{
	std::vector<Item *> out;
	out.reserve(GRID_SIZE + HOTBAR_SIZE + EQUIPMENT_SIZE);

	auto push = [&](const std::unique_ptr<Item> &ptr) {
		if (ptr)
			out.push_back(ptr.get());
	};

	push(hatSlot);
	push(gumSlot);
	push(backupSlot);

	for (auto &i : grid)
		push(i);

	for (auto &i : hotbar)
		push(i);

	return out;
}

json Inventory::serialize() const
{
	json j;

	j["hatSlot"] = hatSlot ? hatSlot->serialize() : nullptr;
	j["gumSlot"] = gumSlot ? gumSlot->serialize() : nullptr;
	j["backupSlot"] = backupSlot ? backupSlot->serialize() : nullptr;

	j["grid"] = json::array();
	for (const auto &slot : grid)
		j["grid"].push_back(slot ? slot->serialize() : nullptr);

	j["hotbar"] = json::array();
	for (const auto &slot : hotbar)
		j["hotbar"].push_back(slot ? slot->serialize() : nullptr);

	return j;
}

void Inventory::deserialize(const json &j)
{
	// --- single slots ---
	if (j.contains("hatSlot") && !j["hatSlot"].is_null())
		hatSlot = ItemFactory::create(j["hatSlot"]);
	else
		hatSlot.reset();

	if (j.contains("gumSlot") && !j["gumSlot"].is_null())
		gumSlot = ItemFactory::create(j["gumSlot"]);
	else
		gumSlot.reset();

	if (j.contains("backupSlot") && !j["backupSlot"].is_null())
		backupSlot = ItemFactory::create(j["backupSlot"]);
	else
		backupSlot.reset();

	for (auto &slot : grid)
		slot.reset();
	if (j.contains("grid")) {
		size_t i = 0;
		for (const auto &slotJson : j["grid"]) {
			if (i >= grid.size())
				break;

			if (!slotJson.is_null())
				grid[i] = ItemFactory::create(slotJson);
			else
				grid[i] = nullptr;

			++i;
		}
	}

	for (auto &slot : hotbar)
		slot.reset();
	if (j.contains("hotbar")) {
		size_t i = 0;
		for (const auto &slotJson : j["hotbar"]) {
			if (i >= hotbar.size())
				break;

			if (!slotJson.is_null())
				hotbar[i] = ItemFactory::create(slotJson);
			else
				hotbar[i] = nullptr;

			++i;
		}
	}
}
