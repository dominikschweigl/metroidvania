#include "combat_system.h"

void CombatSystem::resolve(std::span<const Hitbox> hitboxes, std::span<const Hurtbox> hurtboxes)
{
	for (const Hitbox &hit : hitboxes) {
		for (const Hurtbox &hurt : hurtboxes) {
			if (hit.team == hurt.team)
				continue;
			if (hurt.invulnerable)
				continue;
			if (hurt.health == nullptr)
				continue;
			if (!intersects(hit, hurt))
				continue;

			const auto key = std::pair{hit.sourceId, static_cast<const Health *>(hurt.health)};
			const auto [it, inserted] = resolvedHits.insert(key);
			if (!inserted)
				continue;

			hurt.health->damage(hit.damage);
		}
	}
}

void CombatSystem::clearSource(std::uint32_t sourceId)
{
	std::erase_if(resolvedHits, [sourceId](const auto &pair) { return pair.first == sourceId; });
}

void CombatSystem::clearVictim(const Health *health) noexcept
{
	std::erase_if(resolvedHits, [health](const auto &pair) { return pair.second == health; });
}

void CombatSystem::clear() noexcept
{
	resolvedHits.clear();
}
