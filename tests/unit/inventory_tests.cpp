#include "entities/player/inventory.h"
#include "items/chewing_gum_item.h"
#include "items/hat_item.h"
#include "items/healing_potion_item.h"
#include "items/item_registry.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Inventory: addItem routes Hat to hatSlot", "[inventory]")
{
	Inventory inv;
	REQUIRE(!inv.hasHat());
	inv.addItem(std::make_unique<HatItem>());
	REQUIRE(inv.hasHat());
	REQUIRE(inv.hatSlot != nullptr);
	REQUIRE(inv.hatSlot->equipmentSlot() == SlotKind::Hat);
}

TEST_CASE("Inventory: addItem routes ChewingGum to gumSlot", "[inventory]")
{
	Inventory inv;
	REQUIRE(!inv.hasGum());
	inv.addItem(std::make_unique<ChewingGumItem>());
	REQUIRE(inv.hasGum());
	REQUIRE(inv.gumSlot->equipmentSlot() == SlotKind::Gum);
}

TEST_CASE("Inventory: addItem places HealingPotion in grid", "[inventory]")
{
	Inventory inv;
	inv.addItem(std::make_unique<HealingPotionItem>());
	REQUIRE(inv.hasItem({SlotKind::Grid, 0}));
	REQUIRE(!inv.itemAt({SlotKind::Grid, 0}).equipmentSlot().has_value());
}

TEST_CASE("Inventory: Hat goes to grid if hatSlot already occupied", "[inventory]")
{
	Inventory inv;
	inv.addItem(std::make_unique<HatItem>());
	inv.addItem(std::make_unique<HatItem>());
	REQUIRE(inv.hasHat());
	REQUIRE(inv.hasItem({SlotKind::Grid, 0}));
	REQUIRE(inv.itemAt({SlotKind::Grid, 0}).equipmentSlot() == SlotKind::Hat);
}

TEST_CASE("Inventory: hasItem returns false for empty slot", "[inventory]")
{
	Inventory inv;
	REQUIRE(!inv.hasItem({SlotKind::Hat, 0}));
	REQUIRE(!inv.hasItem({SlotKind::Grid, 3}));
	REQUIRE(!inv.hasItem({SlotKind::Hotbar, 2}));
}

TEST_CASE("Inventory: moveItem moves potion from grid to hotbar", "[inventory]")
{
	Inventory inv;
	inv.addItem(std::make_unique<HealingPotionItem>());
	const bool moved = inv.moveItem({SlotKind::Grid, 0}, {SlotKind::Hotbar, 1});
	REQUIRE(moved);
	REQUIRE(!inv.hasItem({SlotKind::Grid, 0}));
	REQUIRE(inv.hasItem({SlotKind::Hotbar, 1}));
}

TEST_CASE("Inventory: moveItem rejects non-Hat into hatSlot", "[inventory]")
{
	Inventory inv;
	inv.addItem(std::make_unique<HealingPotionItem>());
	const bool moved = inv.moveItem({SlotKind::Grid, 0}, {SlotKind::Hat, 0});
	REQUIRE(!moved);
	REQUIRE(inv.hasItem({SlotKind::Grid, 0}));
	REQUIRE(!inv.hasItem({SlotKind::Hat, 0}));
}

TEST_CASE("Inventory: moveItem swaps items between two grid slots", "[inventory]")
{
	Inventory inv;
	inv.addItem(std::make_unique<HealingPotionItem>());
	inv.addItem(std::make_unique<ChewingGumItem>()); // goes to gumSlot
	// Move a Hat to grid[0] manually via hatSlot then to grid[1]
	inv.addItem(std::make_unique<HatItem>()); // hatSlot
	inv.moveItem({SlotKind::Hat, 0}, {SlotKind::Grid, 1});
	// Now grid[0]=HealingPotion, grid[1]=Hat
	const bool swapped = inv.moveItem({SlotKind::Grid, 0}, {SlotKind::Grid, 1});
	REQUIRE(swapped);
	REQUIRE(inv.itemAt({SlotKind::Grid, 0}).equipmentSlot() == SlotKind::Hat);
	REQUIRE(!inv.itemAt({SlotKind::Grid, 1}).equipmentSlot().has_value());
}

TEST_CASE("Inventory: clearSlot empties the slot", "[inventory]")
{
	Inventory inv;
	inv.addItem(std::make_unique<HealingPotionItem>());
	REQUIRE(inv.hasItem({SlotKind::Grid, 0}));
	inv.clearSlot({SlotKind::Grid, 0});
	REQUIRE(!inv.hasItem({SlotKind::Grid, 0}));
}

TEST_CASE("Inventory: moveToEquipmentSlot returns Hat to hatSlot", "[inventory]")
{
	Inventory inv;
	// Force Hat into grid by filling hatSlot first, then clearing it
	inv.addItem(std::make_unique<HatItem>());
	inv.moveItem({SlotKind::Hat, 0}, {SlotKind::Grid, 0});
	REQUIRE(!inv.hasHat());
	REQUIRE(inv.hasItem({SlotKind::Grid, 0}));

	inv.moveToEquipmentSlot({SlotKind::Grid, 0});
	REQUIRE(inv.hasHat());
	REQUIRE(!inv.hasItem({SlotKind::Grid, 0}));
}

TEST_CASE("Inventory: item info is non-empty for all registered items", "[inventory]")
{
	for (const Item &item : registeredItems())
		REQUIRE(!item.info().name.empty());
}

namespace {
void fillGridAndHotbar(Inventory &inv)
{
	for (int i = 0; i < Inventory::GRID_SIZE + Inventory::HOTBAR_SIZE; ++i)
		inv.addItem(std::make_unique<HealingPotionItem>());
}
} // namespace

TEST_CASE("Inventory: addItem falls through to hotbar when grid is full", "[inventory]")
{
	Inventory inv;
	for (int i = 0; i < Inventory::GRID_SIZE; ++i)
		inv.addItem(std::make_unique<HealingPotionItem>());
	REQUIRE(!inv.hasItem({SlotKind::Hotbar, 0}));
	inv.addItem(std::make_unique<HealingPotionItem>());
	REQUIRE(inv.hasItem({SlotKind::Hotbar, 0}));
}

TEST_CASE("Inventory: addItem fills all hotbar slots sequentially after grid is full", "[inventory]")
{
	Inventory inv;
	fillGridAndHotbar(inv);
	for (int i = 0; i < Inventory::HOTBAR_SIZE; ++i)
		REQUIRE(inv.hasItem({SlotKind::Hotbar, i}));
}

TEST_CASE("Inventory: addItem does not place item when grid and hotbar are both full", "[inventory]")
{
	Inventory inv;
	fillGridAndHotbar(inv);
	REQUIRE(inv.flatten().size() == Inventory::GRID_SIZE + Inventory::HOTBAR_SIZE);
	inv.addItem(std::make_unique<HealingPotionItem>());
	REQUIRE(inv.flatten().size() == Inventory::GRID_SIZE + Inventory::HOTBAR_SIZE);
}

TEST_CASE("Inventory: canAdd returns true when inventory has free grid slots", "[inventory]")
{
	const Inventory inv;
	const HealingPotionItem potion;
	REQUIRE(inv.canAdd(potion));
}

TEST_CASE("Inventory: canAdd returns true for regular item when grid is full but hotbar has space", "[inventory]")
{
	Inventory inv;
	for (int i = 0; i < Inventory::GRID_SIZE; ++i)
		inv.addItem(std::make_unique<HealingPotionItem>());
	const HealingPotionItem potion;
	REQUIRE(inv.canAdd(potion));
}

TEST_CASE("Inventory: canAdd returns false for regular item when grid and hotbar are both full", "[inventory]")
{
	Inventory inv;
	fillGridAndHotbar(inv);
	const HealingPotionItem potion;
	REQUIRE_FALSE(inv.canAdd(potion));
}

TEST_CASE("Inventory: canAdd returns true for Hat when hatSlot is free even if grid and hotbar are full", "[inventory]")
{
	Inventory inv;
	fillGridAndHotbar(inv);
	const HatItem hat;
	REQUIRE(inv.canAdd(hat));
}

TEST_CASE("Inventory: canAdd returns false for Hat when hatSlot grid and hotbar are all full", "[inventory]")
{
	Inventory inv;
	inv.addItem(std::make_unique<HatItem>());
	fillGridAndHotbar(inv);
	const HatItem hat;
	REQUIRE_FALSE(inv.canAdd(hat));
}
