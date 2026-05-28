#pragma once

#include "hitbox.h"
#include <cstdint>
#include <set>
#include <span>
#include <utility>

// Central combat resolver.
// Resolves every Hitbox against every Hurtbox of the opposite team. Each
// (sourceId, victim) pair damages at most once. Invulnerable hurtboxes are skipped.
class CombatSystem {
  public:
	CombatSystem() = default;
	~CombatSystem() = default;
	CombatSystem(const CombatSystem &) = delete;
	CombatSystem &operator=(const CombatSystem &) = delete;
	CombatSystem(CombatSystem &&) = delete;
	CombatSystem &operator=(CombatSystem &&) = delete;

	void resolve(std::span<const Hitbox> hitboxes, std::span<const Hurtbox> hurtboxes);

	// Clear every recorded (sourceId, victim) pair for the given source.
	void clearSource(std::uint32_t sourceId);

	// Clear every recorded (sourceId, victim) pair for the given victim.
	void clearVictim(const Health *health) noexcept;

	// Clear every recorded hit. Used for tests / scene resets.
	void clear() noexcept;

  private:
	std::set<std::pair<std::uint32_t, const Health *>> resolvedHits;
};
