#include "items/healing_potion_item.h"
#include "items/world_item.h"
#include "world/world.h"
#include <catch2/catch_test_macros.hpp>

namespace {
constexpr sf::FloatRect intersectingBounds{{-100.f, -100.f}, {200.f, 1000.f}};
// Far away — guaranteed no intersection.
constexpr sf::FloatRect nonIntersectingBounds{{1000.f, 1000.f}, {10.f, 10.f}};

// Simulates a single frame past the pickup cooldown using an empty world
// (no solid tiles, so the item falls freely).
void advancePastCooldown(WorldItem &worldItem)
{
	const World world("test");
	worldItem.update(WorldItem::PICKUP_COOLDOWN + 0.01f, world);
}
} // namespace

TEST_CASE("WorldItem: peekItem has value when item is present", "[worlditem]")
{
	WorldItem worldItem(sf::Vector2f{0.f, 0.f}, sf::Vector2f{0.f, 0.f}, std::make_unique<HealingPotionItem>());
	const std::optional<std::reference_wrapper<const Item>> peeked = worldItem.peekItem();
	REQUIRE(peeked.has_value());
}

TEST_CASE("WorldItem: peekItem returns nullopt after tryCollect moves the item out", "[worlditem]")
{
	WorldItem worldItem(sf::Vector2f{0.f, 0.f}, sf::Vector2f{0.f, 0.f}, std::make_unique<HealingPotionItem>());
	advancePastCooldown(worldItem);
	const std::unique_ptr<Item> collected = worldItem.tryCollect(intersectingBounds);
	REQUIRE(collected != nullptr);
	const std::optional<std::reference_wrapper<const Item>> peeked = worldItem.peekItem();
	REQUIRE_FALSE(peeked.has_value());
}

TEST_CASE("WorldItem: peekItem still has value when tryCollect found no intersection", "[worlditem]")
{
	WorldItem worldItem(sf::Vector2f{0.f, 0.f}, sf::Vector2f{0.f, 0.f}, std::make_unique<HealingPotionItem>());
	advancePastCooldown(worldItem);
	const std::unique_ptr<Item> collected = worldItem.tryCollect(nonIntersectingBounds);
	REQUIRE(collected == nullptr);
	const std::optional<std::reference_wrapper<const Item>> peeked = worldItem.peekItem();
	REQUIRE(peeked.has_value());
}

TEST_CASE("WorldItem: peekItem returns nullopt when already marked collected", "[worlditem]")
{
	WorldItem worldItem(sf::Vector2f{0.f, 0.f}, sf::Vector2f{0.f, 0.f}, std::make_unique<HealingPotionItem>());
	advancePastCooldown(worldItem);
	const std::unique_ptr<Item> first = worldItem.tryCollect(intersectingBounds);
	REQUIRE(first != nullptr);
	// A second tryCollect on an already-collected item returns nullptr.
	const std::unique_ptr<Item> second = worldItem.tryCollect(intersectingBounds);
	REQUIRE(second == nullptr);
	REQUIRE_FALSE(worldItem.peekItem().has_value());
}

TEST_CASE("WorldItem: tryCollect returns nullptr while pickup cooldown is active", "[worlditem]")
{
	WorldItem worldItem(sf::Vector2f{0.f, 0.f}, sf::Vector2f{0.f, 0.f}, std::make_unique<HealingPotionItem>());
	const World world("test");
	worldItem.update(WorldItem::PICKUP_COOLDOWN * 0.5f, world);
	const std::unique_ptr<Item> collected = worldItem.tryCollect(intersectingBounds);
	REQUIRE(collected == nullptr);
	REQUIRE(worldItem.peekItem().has_value());
}

TEST_CASE("WorldItem: tryCollect returns item once pickup cooldown has elapsed", "[worlditem]")
{
	WorldItem worldItem(sf::Vector2f{0.f, 0.f}, sf::Vector2f{0.f, 0.f}, std::make_unique<HealingPotionItem>());
	const World world("test");
	worldItem.update(WorldItem::PICKUP_COOLDOWN + 0.01f, world);
	const std::unique_ptr<Item> collected = worldItem.tryCollect(intersectingBounds);
	REQUIRE(collected != nullptr);
}
