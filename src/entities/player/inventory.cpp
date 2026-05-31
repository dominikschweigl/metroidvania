#include "inventory.h"
#include "player.h"
#include <stdexcept>

bool Inventory::isValidInSlot(const Item &item, const SlotKind slotKind) noexcept
{
	const std::optional<SlotKind> equipment = item.equipmentSlot();
	if (slotKind == SlotKind::Hat || slotKind == SlotKind::Gum)
		return equipment.has_value() && *equipment == slotKind;
	return true;
}

std::unique_ptr<Item> &Inventory::slotRef(const SlotRef slot)
{
	switch (slot.kind) {
	case SlotKind::Hat:
		return hatSlot;
	case SlotKind::Gum:
		return gumSlot;
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
