#include "segfault_boss.h"
#include "../../../../world/world.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace {

[[nodiscard]] float findGroundY(const World &world, float worldX, float fromY)
{
	constexpr float TILE = World::TILE_SIZE;
	constexpr int MAX_ROWS = 64;

	const float baseY = std::floor(fromY / TILE) * TILE;
	for (int row = 0; row < MAX_ROWS; ++row) {
		const float top = baseY + static_cast<float>(row) * TILE;
		const sf::FloatRect cell({worldX - 1.f, top + TILE * 0.5f}, {2.f, 2.f});
		if (world.isSolidAtRect(cell))
			return top;
	}
	return fromY;
}
} // namespace

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

	// NULL spears: a ground marker while telegraphed, the spear itself while striking.
	for (const NullSpear &spear : spears) {
		if (spear.phase == SpearPhase::Windup)
			renderer.drawSpearTelegraph(window, spear.foot, SPEAR_WIDTH, effectTimer);
		else
			renderer.drawSpear(window, spear.foot, SPEAR_WIDTH, SPEAR_HEIGHT, effectTimer);
	}
}

void SegfaultBoss::onHit(const Hitbox & /*hit*/) noexcept
{
	triggerHurtFlash();
}

void SegfaultBoss::onPreUpdate(float deltaTime)
{
	// Defeat: drop any active threat and switch to the death state.
	if (!dying && health.current <= 0) {
		dying = true;
		deathFacing = direction;
		for (const NullSpear &spear : spears)
			if (spear.sourceId != 0)
				endedSourceIds.push_back(spear.sourceId);
		spears.clear();

		currentState->onExit(*this);
		currentState = &states.death;
		currentState->onEnter(*this);
		return;
	}

	if (dying)
		return;

	spearCooldown = std::max(0.f, spearCooldown - deltaTime);
	effectTimer += deltaTime;

	// Advance each spear through its windup/strike lifecycle, retiring spent ones.
	std::erase_if(spears, [this, deltaTime](NullSpear &spear) {
		spear.timer += deltaTime;
		if (spear.phase == SpearPhase::Windup) {
			if (spear.timer >= SPEAR_WINDUP_DUR) {
				spear.phase = SpearPhase::Strike;
				spear.timer = 0.f;
				spear.sourceId = nextSourceId();
			}
			return false;
		}
		if (spear.timer >= SPEAR_STRIKE_DUR) {
			endedSourceIds.push_back(spear.sourceId);
			return true;
		}
		return false;
	});
}

void SegfaultBoss::setAnimation(const SegfaultBossAnimation anim, const int frame)
{
	renderer.setAnimation(anim, frame);
}

void SegfaultBoss::spawnNullSpear(const float worldX, const float fromY, const World &world)
{
	const float groundY = findGroundY(world, worldX, fromY);
	spears.push_back(NullSpear{{worldX, groundY}, SpearPhase::Windup, 0.f, 0});
}

void SegfaultBoss::spawnNullSpearOnPlayer()
{
	if (lastWorld != nullptr)
		spawnNullSpear(lastPlayerPos.x, lastPlayerPos.y, *lastWorld);
}

void SegfaultBoss::collectHitboxes(std::vector<Hitbox> &hitboxes)
{
	for (const NullSpear &spear : spears) {
		if (spear.phase != SpearPhase::Strike)
			continue;
		const sf::FloatRect bounds({spear.foot.x - SPEAR_WIDTH / 2.f, spear.foot.y - SPEAR_HEIGHT},
		                           {SPEAR_WIDTH, SPEAR_HEIGHT});
		hitboxes.push_back(Hitbox{bounds, SPEAR_DAMAGE, Team::Enemy, spear.sourceId});
	}
}

void SegfaultBoss::drainEndedSourceIds(std::vector<std::uint32_t> &out)
{
	out.insert(out.end(), endedSourceIds.begin(), endedSourceIds.end());
	endedSourceIds.clear();
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
