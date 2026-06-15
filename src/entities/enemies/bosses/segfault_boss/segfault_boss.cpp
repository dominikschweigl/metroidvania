#include "segfault_boss.h"

SegfaultBoss::SegfaultBoss(sf::Vector2f spawnPos) : BaseEnemy(spawnPos, ENTITY_WIDTH, ENTITY_HEIGHT, BOSS_HEALTH)
{
	currentState = &states.roaming;
}

void SegfaultBoss::draw(sf::RenderWindow &window)
{
	const Direction facing = dying ? deathFacing : direction;
	const float scaleX = (facing == Direction::Right) ? 1.f : -1.f;

	// Placeholder until final sprites are created
	const sf::Color tint = dying              ? sf::Color{120, 120, 120}
	                       : isHurtFlashing() ? sf::Color{255, 80, 80}
	                       : invincible       ? sf::Color{150, 220, 255}
	                                          : sf::Color{230, 140, 255};
	renderer.drawSprite(window, position, scaleX, tint);
}

void SegfaultBoss::onHit(const Hitbox & /*hit*/) noexcept
{
	triggerHurtFlash();
}

void SegfaultBoss::onPreUpdate(float /*deltaTime*/)
{
	// Defeat: switch to the death state and stay there.
	if (!dying && health.current <= 0) {
		dying = true;
		deathFacing = direction;

		currentState->onExit(*this);
		currentState = &states.death;
		currentState->onEnter(*this);
		return;
	}
}

void SegfaultBoss::setAnimation(const SegfaultBossAnimation anim, const int frame)
{
	renderer.setAnimation(anim, frame);
}

json SegfaultBoss::serialize() const
{
	json j = BaseEnemy::serialize();
	j["type"] = "SegfaultBoss";
	return j;
}

void SegfaultBoss::deserialize(const json &j)
{
	BaseEnemy::deserialize(j);
}
