#include "base_entity.h"

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
	return Hurtbox{getBounds(), team, &health, isInvulnerable()};
}

void BaseEntity::collectHitboxes(std::vector<Hitbox> & /*hitboxes*/) {}

void BaseEntity::collectHurtboxes(std::vector<Hurtbox> &hurtboxes)
{
	hurtboxes.push_back(getHurtbox());
}
