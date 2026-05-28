#pragma once

#include <algorithm>

// Health struct to be used by the player and enemies.
// Damage and heal are clamped.
struct Health {
	int max;
	int current;

	[[nodiscard]] bool isAlive() const noexcept { return current > 0; }

	void damage(int amount) noexcept
	{
		if (amount <= 0)
			return;
		current = std::max(0, current - amount);
	}

	void heal(int amount) noexcept
	{
		if (amount <= 0)
			return;
		current = std::min(max, current + amount);
	}
};
