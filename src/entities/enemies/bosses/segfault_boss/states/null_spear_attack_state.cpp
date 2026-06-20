#include "null_spear_attack_state.h"
#include "../segfault_boss.h"

#include <cstdlib>

namespace segfault_boss {

namespace {
[[nodiscard]] float randomRange(float low, float high)
{
	return low + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * (high - low);
}
} // namespace

EnemyState *NullSpearAttackState::update(float deltaTime, BaseEnemy &enemy, const World &world, sf::Vector2f playerPos)
{
	auto &boss = static_cast<SegfaultBoss &>(enemy);

	if (spawned < spawnTarget) {
		spawnTimer += deltaTime;
		if (spawnTimer >= SegfaultBoss::SPEAR_SPAWN_INTERVAL) {
			spawnTimer -= SegfaultBoss::SPEAR_SPAWN_INTERVAL;
			const bool tracksPlayer = (spawned % 2 == 1);
			const float offsetX =
			    tracksPlayer ? 0.f : randomRange(-SegfaultBoss::SPEAR_SPREAD, SegfaultBoss::SPEAR_SPREAD);
			boss.spawnNullSpear(playerPos.x + offsetX, playerPos.y, world);
			++spawned;
		}
		return this;
	}

	if (!boss.hasActiveSpears())
		return &boss.states.recover;

	return this;
}

void NullSpearAttackState::updateAnimation(float deltaTime, BaseEnemy &enemy)
{
	auto &boss = static_cast<SegfaultBoss &>(enemy);

	constexpr int FRAME_COUNT = 12;
	constexpr float FRAME_DURATION = 0.1f;

	frameTimer += deltaTime;
	if (frameTimer >= FRAME_DURATION) {
		frameTimer -= FRAME_DURATION;
		currentFrame = (currentFrame + 1) % FRAME_COUNT;
	}

	boss.setAnimation(SegfaultBoss::SegfaultBossAnimation::Attack, currentFrame);
}

void NullSpearAttackState::onEnter(BaseEnemy &enemy)
{
	auto &boss = static_cast<SegfaultBoss &>(enemy);
	boss.setVelocityX(0.f);
	boss.startSpearCooldown();

	spawnTarget =
	    SegfaultBoss::SPEAR_MIN_COUNT + rand() % (SegfaultBoss::SPEAR_MAX_COUNT - SegfaultBoss::SPEAR_MIN_COUNT + 1);

	boss.spawnNullSpearOnPlayer();
	spawned = 1;
	spawnTimer = 0.f;
	currentFrame = 0;
	frameTimer = 0.f;
}

void NullSpearAttackState::onExit(BaseEnemy & /*enemy*/)
{
	spawnTarget = 0;
	spawned = 0;
	spawnTimer = 0.f;
}

json NullSpearAttackState::serialize() const
{
	json j = EnemyState::serialize();
	j["type"] = "NullSpearAttackState";
	return j;
}

} // namespace segfault_boss
