#include "items/healing_potion_item.h"
#include "items/world_item.h"
#include <catch2/catch_test_macros.hpp>

namespace {
// Large enough to always intersect a WorldItem at (0, 0).
constexpr sf::FloatRect intersectingBounds{{-100.f, -100.f}, {200.f, 200.f}};
// Far away — guaranteed no intersection.
constexpr sf::FloatRect nonIntersectingBounds{{1000.f, 1000.f}, {10.f, 10.f}};
} // namespace

TEST_CASE("WorldItem: peekItem has value when item is present", "[worlditem]")
{
	WorldItem worldItem(sf::Vector2f{0.f, 0.f}, std::make_unique<HealingPotionItem>());
	const std::optional<std::reference_wrapper<const Item>> peeked = worldItem.peekItem();
	REQUIRE(peeked.has_value());
}

TEST_CASE("WorldItem: peekItem returns nullopt after tryCollect moves the item out", "[worlditem]")
{
	WorldItem worldItem(sf::Vector2f{0.f, 0.f}, std::make_unique<HealingPotionItem>());
	const std::unique_ptr<Item> collected = worldItem.tryCollect(intersectingBounds);
	REQUIRE(collected != nullptr);
	const std::optional<std::reference_wrapper<const Item>> peeked = worldItem.peekItem();
	REQUIRE_FALSE(peeked.has_value());
}

TEST_CASE("WorldItem: peekItem still has value when tryCollect found no intersection", "[worlditem]")
{
	WorldItem worldItem(sf::Vector2f{0.f, 0.f}, std::make_unique<HealingPotionItem>());
	const std::unique_ptr<Item> collected = worldItem.tryCollect(nonIntersectingBounds);
	REQUIRE(collected == nullptr);
	const std::optional<std::reference_wrapper<const Item>> peeked = worldItem.peekItem();
	REQUIRE(peeked.has_value());
}

TEST_CASE("WorldItem: peekItem returns nullopt when already marked collected", "[worlditem]")
{
	WorldItem worldItem(sf::Vector2f{0.f, 0.f}, std::make_unique<HealingPotionItem>());
	const std::unique_ptr<Item> first = worldItem.tryCollect(intersectingBounds);
	REQUIRE(first != nullptr);
	// A second tryCollect on an already-collected item returns nullptr.
	const std::unique_ptr<Item> second = worldItem.tryCollect(intersectingBounds);
	REQUIRE(second == nullptr);
	REQUIRE_FALSE(worldItem.peekItem().has_value());
}
