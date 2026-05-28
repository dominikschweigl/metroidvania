#pragma once
#include "../../../combat/hitbox.h"
#include "../../direction.h"
#include <SFML/Graphics.hpp>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

struct AttackDef {
	std::reference_wrapper<const sf::Texture> upperTexture;
	int frameCount;
	float frameDuration;
};

class MeleeAttack {
  public:
	static constexpr int DAMAGE = 1;
	static constexpr float HITBOX_SIZE = 32.f;

	const sf::Texture &swing_texture;
	const sf::Texture &overhead_texture;

	MeleeAttack();

	[[nodiscard]] bool isMeleeActive() const noexcept { return comboIndex >= 0; }
	[[nodiscard]] std::uint32_t getSourceId() const noexcept { return sourceId; }

	void reset() noexcept;
	void trigger();
	void update(float dt);
	void applyAnimation(sf::Sprite &upper, sf::Vector2f scale, sf::Vector2f pos) const;

	// Append sourceIds whose combo step ended since the last drain.
	void drainEndedSourceIds(std::vector<std::uint32_t> &out) noexcept;

	// Returns the active damage rectangle for the swing, or nullopt if no swing
	// is active. `playerPos` is the player's foot-centered position. `facing`
	// decides which side of the body the hitbox is positioned on.
	[[nodiscard]] std::optional<Hitbox> getHitbox(sf::Vector2f playerPos, Direction facing) const noexcept;

  private:
	std::vector<AttackDef> comboChain;

	int comboIndex = -1;
	int frame = 0;
	float frameTimer = 0.f;
	bool comboQueued = false;
	std::uint32_t sourceId = 0;
	std::vector<std::uint32_t> endedSourceIds;
};
