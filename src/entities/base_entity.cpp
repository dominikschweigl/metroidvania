#include "base_entity.h"

#include <algorithm>

BaseEntity::BaseEntity(sf::Vector2f spawnPos, float entityWidth, float entityHeight, int maxHealth, Team entityTeam,
                       float entityGravity)
    : health{maxHealth, maxHealth}, gravity(entityGravity), position(spawnPos), width(entityWidth),
      height(entityHeight), team(entityTeam)
{
}

sf::FloatRect BaseEntity::getBounds() const
{
	return sf::FloatRect({position.x - width / 2.f, position.y - height}, {width, height});
}

Hurtbox BaseEntity::getHurtbox() noexcept
{
	return Hurtbox{getBounds(), team, &health, isInvulnerable(), this};
}

void BaseEntity::collectHitboxes(std::vector<Hitbox> & /*hitboxes*/) {}

void BaseEntity::collectHurtboxes(std::vector<Hurtbox> &hurtboxes)
{
	hurtboxes.push_back(getHurtbox());
}

void BaseEntity::tickHurtTimers(float deltaTime) noexcept
{
	knockbackTimer = std::max(0.f, knockbackTimer - deltaTime);
	hurtFlashTimer = std::max(0.f, hurtFlashTimer - deltaTime);
}

void BaseEntity::onHit(const Hitbox &hit) noexcept
{
	const float hitCenterX = hit.bounds.position.x + hit.bounds.size.x / 2.f;
	const float pushSign = (hitCenterX >= position.x) ? -1.f : 1.f;

	velocity.x = pushSign * KNOCKBACK_X_SPEED;
	velocity.y = -KNOCKBACK_Y_SPEED;
	isOnGround = false;

	knockbackTimer = KNOCKBACK_DURATION;
	hurtFlashTimer = HURT_FLASH_DURATION;
}

json BaseEntity::serialize() const
{
	return {{"position", {position.x, position.y}},
	        {"velocity", {velocity.x, velocity.y}},
	        {"direction", direction == Direction::Left ? "Left" : "Right"},
	        {"isOnGround", isOnGround},
	        {"health", health.serialize()},
	        {"gravity", gravity}};
}

void BaseEntity::deserialize(const json &j)
{
	if (j.contains("position")) {
		position.x = j["position"][0];
		position.y = j["position"][1];
	}

	if (j.contains("velocity")) {
		velocity.x = j["velocity"][0];
		velocity.y = j["velocity"][1];
	}

	if (j.contains("direction")) {
		direction = (j["direction"] == "Left") ? Direction::Left : Direction::Right;
	}

	if (j.contains("isOnGround")) {
		isOnGround = j["isOnGround"];
	}

	if (j.contains("health")) {
		health.deserialize(j["health"]);
	}

	if (j.contains("gravity")) {
		gravity = j["gravity"];
	}
}
