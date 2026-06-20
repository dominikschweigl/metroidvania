#pragma once

#include "health.h"
#include <SFML/Graphics.hpp>
#include <cstdint>

class BaseEntity;

enum class Team : std::uint8_t { Player, Enemy };

enum class StatusEffectKind : std::uint8_t { None, Slow };

// Damage dealing rectangle.
// sourceId identifies the attack instance so the central
// CombatSystem applies damage only once across frames.
struct Hitbox {
	sf::FloatRect bounds;
	int damage;
	Team team;
	std::uint32_t sourceId;
	StatusEffectKind statusOnHit = StatusEffectKind::None;
};

// Damage receiving rectangle.
// CombatSystem notifies target via *health about damage.
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

// New attacks receive increasing ids to be identifiable by
// combat manager between frames in order to apply damage only once.
[[nodiscard]] inline std::uint32_t nextSourceId() noexcept
{
	static std::uint32_t counter = 0;
	return ++counter;
}
