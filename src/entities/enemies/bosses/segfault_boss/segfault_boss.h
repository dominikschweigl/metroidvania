#pragma once

#include "../../base_enemy.h"

#include "segfault_boss_renderer.h"

#include "states/death_state.h"
#include "states/roaming_state.h"

// Second-area boss: a corrupted scientist whose process is leaking memory.
class SegfaultBoss : public BaseEnemy {
  public:
	using SegfaultBossAnimation = segfault_boss::SegfaultBossRenderer::Animation;

	static constexpr float ENTITY_WIDTH = 40.f;
	static constexpr float ENTITY_HEIGHT = 64.f;

	static constexpr int BOSS_HEALTH = 36;

	static constexpr float MOVE_SPEED = 70.f;
	static constexpr float MOVE_DIRECTION_DUR = 5.f;

	struct States {
		segfault_boss::RoamingState roaming;
		segfault_boss::DeathState death;
	};
	States states;

	explicit SegfaultBoss(sf::Vector2f spawnPos);

	void draw(sf::RenderWindow &window) override;

	void onHit(const Hitbox &hit) noexcept override;

	void setAnimation(SegfaultBossAnimation anim, int frame);

	void setInvincible(bool value) noexcept { invincible = value; }
	[[nodiscard]] bool isInvulnerable() const noexcept override { return invincible; }

	// Ability to request a "bluescreen" between stages 2 and 3.
	[[nodiscard]] bool consumeBluescreenRequest() noexcept
	{
		const bool requested = bluescreenRequested;
		bluescreenRequested = false;
		return requested;
	}

	[[nodiscard]] bool isAlive() const noexcept override { return health.isAlive() || dying; }

	json serialize() const override;
	void deserialize(const json &j) override;

  protected:
	void onPreUpdate(float deltaTime) override;

  private:
	segfault_boss::SegfaultBossRenderer renderer;

	bool invincible = false;
	bool dying = false;
	// Raised when crossing into stage three so the scene can show the bluescreen.
	bool bluescreenRequested = false;
	Direction deathFacing = Direction::Right;
};
