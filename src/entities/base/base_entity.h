#pragma once

#include "../../combat/health.h"
#include "../../combat/hitbox.h"
#include "../../core/direction.h"
#include <SFML/Graphics.hpp>
#include <vector>

// Shared base for every gameplay entity (player and enemies).
// Owns position, velocity, facing direction, ground-flag, health, body dimensions,
// gravity, and team.
class BaseEntity {
  public:
	virtual ~BaseEntity() = default;

	// Derived Entity should not be copyable since it would share it's stats (eg health)
	BaseEntity(const BaseEntity &) = delete;
	BaseEntity &operator=(const BaseEntity &) = delete;
	BaseEntity(BaseEntity &&) = delete;
	BaseEntity &operator=(BaseEntity &&) = delete;

	[[nodiscard]] sf::Vector2f getPosition() const noexcept { return position; }
	void setPosition(sf::Vector2f newPosition) noexcept { position = newPosition; }

	[[nodiscard]] sf::Vector2f getVelocity() const noexcept { return velocity; }
	void setVelocity(sf::Vector2f newVelocity) noexcept { velocity = newVelocity; }
	void setVelocityX(float vx) noexcept { velocity.x = vx; }
	void setVelocityY(float vy) noexcept { velocity.y = vy; }
	void resetVelocity() noexcept { velocity = sf::Vector2f{0.f, 0.f}; }

	[[nodiscard]] Direction getDirection() const noexcept { return direction; }
	void setDirection(Direction d) noexcept { direction = d; }

	[[nodiscard]] bool isOnGroundFlag() const noexcept { return isOnGround; }
	void setOnGround(bool onGround) noexcept { isOnGround = onGround; }

	[[nodiscard]] virtual sf::FloatRect getBounds() const;

	[[nodiscard]] bool isAlive() const noexcept { return health.isAlive(); }
	void takeDamage(int amount) noexcept { health.damage(amount); }

	// Override to mark the entity as currently invulnerable to incoming damage.
	[[nodiscard]] virtual bool isInvulnerable() const noexcept { return false; }

	[[nodiscard]] virtual Hurtbox getHurtbox() noexcept;

	// Default: entities own no active hitboxes. Override to publish them.
	virtual void collectHitboxes(std::vector<Hitbox> &hitboxes);

	// Default: appends this entity's body hurtbox.
	virtual void collectHurtboxes(std::vector<Hurtbox> &hurtboxes);

	Health health;
	float gravity;

  protected:
	BaseEntity(sf::Vector2f spawnPos, float entityWidth, float entityHeight, int maxHealth, Team entityTeam,
	           float entityGravity = 1200.f);

	sf::Vector2f position;
	sf::Vector2f velocity{0.f, 0.f};
	Direction direction = Direction::Right;
	bool isOnGround = false;
	const float width;
	const float height;
	const Team team;
};
