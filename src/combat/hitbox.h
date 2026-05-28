#pragma once

#include "health.h"
#include <SFML/Graphics.hpp>
#include <cstdint>

class BaseEntity;

enum class Team : std::uint8_t { Player, Enemy };

// An active damage-dealing rectangle for one frame. `sourceId` identifies the
// attack instance (e.g. a single melee swing or hat throw) so the central
// CombatSystem can deduplicate hits across frames.
struct Hitbox {
	sf::FloatRect bounds;
	int damage;
	Team team;
	std::uint32_t sourceId;
};

// A damage-receiving rectangle.
// the CombatSystem uses the health address as the entities identity.
// `owner` is an optional callback target - when set, CombatSystem notifies it
// after damage so the entity can react (hurt flash, knockback).
struct Hurtbox {
	sf::FloatRect bounds;
	Team team;
	Health *health;
	bool invulnerable;
	BaseEntity *owner = nullptr;
};

// Returns true if two rectangles overlap.
[[nodiscard]] inline bool intersects(const Hitbox &hit, const Hurtbox &hurt) noexcept
{
	return hit.bounds.findIntersection(hurt.bounds).has_value();
}

// New attack instances receive increasing ids to be distinguishable.
// Attacks capture one on activation and reuse it across frames
// so the CombatSystem can deduplicate hits per attack.
[[nodiscard]] inline std::uint32_t nextSourceId() noexcept
{
	static std::uint32_t counter = 0;
	return ++counter;
}
