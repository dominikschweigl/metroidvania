#include "segfault_boss.h"
#include "../../../../world/world.h"
#include "../../capacitor/capacitor.h"
#include "../../race_condition_slime/race_condition_slime.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <vector>

namespace {

[[nodiscard]] float randomRange(float low, float high)
{
	return low + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * (high - low);
}

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

SegfaultBoss::SegfaultBoss(sf::Vector2f spawnPos) : SegfaultBoss(spawnPos, false, DROP_CHANCE) {}

SegfaultBoss::SegfaultBoss(sf::Vector2f spawnPos, bool isClone, float drop_chance = DROP_CHANCE)
    : BaseEnemy(spawnPos, ENTITY_WIDTH, ENTITY_HEIGHT, isClone ? CLONE_HEALTH : BOSS_HEALTH, drop_chance),
      cloneProcess(isClone)
{
	currentState = &states.roaming;
}

void SegfaultBoss::draw(sf::RenderWindow &window)
{
	const Direction facing = dying ? deathFacing : direction;
	const float scaleX = (facing == Direction::Right) ? 1.f : -1.f;

	const sf::Color tint = isHurtFlashing() ? sf::Color{255, 80, 80} : sf::Color::White;
	for (const CorruptionBlock &block : corruptionBlocks)
		renderer.drawCorruptionBlock(window, block.bounds, block.age / CORRUPTION_LIFETIME, effectTimer);

	renderer.drawSprite(window, position, scaleX, tint);

	if (forkedClone != nullptr)
		forkedClone->draw(window);

	for (const std::unique_ptr<BaseEnemy> &process : summonedProcesses)
		process->draw(window);

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

		for (std::unique_ptr<BaseEnemy> &process : summonedProcesses)
			process->drainEndedSourceIds(endedSourceIds);
		summonedProcesses.clear();

		for (const CorruptionBlock &block : corruptionBlocks)
			endedSourceIds.push_back(block.sourceId);
		corruptionBlocks.clear();

		if (forkedClone != nullptr) {
			forkedClone->drainEndedSourceIds(endedSourceIds);
			forkedClone = nullptr;
		}

		currentState->onExit(*this);
		currentState = &states.death;
		currentState->onEnter(*this);
		return;
	}

	if (dying)
		return;

	spearCooldown = std::max(0.f, spearCooldown - deltaTime);
	effectTimer += deltaTime;

	// Drive boss-owned summons, retiring any the player has destroyed.
	if (!summonedProcesses.empty()) {
		for (std::unique_ptr<BaseEnemy> &process : summonedProcesses)
			process->update(deltaTime, *lastWorld, lastPlayerPos, lastPlayerBounds);

		std::erase_if(summonedProcesses, [this](const std::unique_ptr<BaseEnemy> &process) {
			if (!process->isAlive()) {
				process->drainEndedSourceIds(endedSourceIds);
				return true;
			}
			return false;
		});
	}

	// Drive the boss-owned clone; retire it once the player has destroyed it. The
	// clone's own isAlive() never goes false (it lingers in a death state), so the
	// health check is what prunes it.
	if (forkedClone != nullptr) {
		forkedClone->update(deltaTime, *lastWorld, lastPlayerPos, lastPlayerBounds);
		if (!forkedClone->health.isAlive()) {
			forkedClone->drainEndedSourceIds(endedSourceIds);
			forkedClone = nullptr;
		}
	}

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

	// Leaking memory: corruption blocks erupt periodically and intensify per stage.
	corruptionSpawnTimer += deltaTime;
	if (corruptionSpawnTimer >= corruptionInterval()) {
		corruptionSpawnTimer = 0.f;
		if (corruptionBlocks.size() < CORRUPTION_MAX_BLOCKS)
			spawnCorruptionBlock();
	}

	std::erase_if(corruptionBlocks, [this, deltaTime](CorruptionBlock &block) {
		block.age += deltaTime;
		if (block.age >= CORRUPTION_LIFETIME) {
			endedSourceIds.push_back(block.sourceId);
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

void SegfaultBoss::spawnSummonedProcesses()
{
	stage = 2;
	stage2Triggered = true;

	for (int index = 0; index < SUMMON_COUNT; ++index) {
		const float offsetX = SUMMON_SPREAD * (static_cast<float>(index) - (SUMMON_COUNT - 1) / 2.f);
		const sf::Vector2f spawn{position.x + offsetX, position.y};

		// Mix area-1 slimes (grounded) and area-2 capacitors (airborne).
		if (rand() % 2 == 0)
			summonedProcesses.push_back(std::make_unique<RaceConditionSlime>(spawn, BaseEnemy::DROP_CHANCE));
		else
			summonedProcesses.push_back(std::make_unique<Capacitor>(sf::Vector2f{spawn.x, spawn.y - SUMMON_AIR_HEIGHT},
			                                                        BaseEnemy::DROP_CHANCE));
	}
}

void SegfaultBoss::spawnFork()
{
	stage = 3;
	stage3Triggered = true;

	const float offsetX = (direction == Direction::Right) ? -FORK_OFFSET_X : FORK_OFFSET_X;
	forkedClone = std::make_unique<SegfaultBoss>(sf::Vector2f{position.x + offsetX, position.y}, true);
}

float SegfaultBoss::corruptionInterval() const noexcept
{
	const float interval = CORRUPTION_INTERVAL_BASE - static_cast<float>(stage - 1) * CORRUPTION_INTERVAL_STEP;
	return std::max(CORRUPTION_INTERVAL_MIN, interval);
}

int SegfaultBoss::corruptionDamage() const noexcept
{
	return CORRUPTION_DAMAGE_BASE + (stage - 1);
}

void SegfaultBoss::spawnCorruptionBlock()
{
	if (lastWorld == nullptr)
		return;

	const float spawnX = lastPlayerPos.x + randomRange(-CORRUPTION_SPREAD, CORRUPTION_SPREAD);
	const float groundY = findGroundY(*lastWorld, spawnX, lastPlayerPos.y);
	const sf::FloatRect bounds({spawnX - CORRUPTION_SIZE / 2.f, groundY - CORRUPTION_SIZE},
	                           {CORRUPTION_SIZE, CORRUPTION_SIZE});
	corruptionBlocks.push_back(CorruptionBlock{bounds, corruptionDamage(), 0.f, nextSourceId()});
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

	for (const CorruptionBlock &block : corruptionBlocks)
		hitboxes.push_back(Hitbox{block.bounds, block.damage, Team::Enemy, block.sourceId});

	for (const std::unique_ptr<BaseEnemy> &process : summonedProcesses)
		process->collectHitboxes(hitboxes);

	if (forkedClone != nullptr)
		forkedClone->collectHitboxes(hitboxes);
}

void SegfaultBoss::collectHurtboxes(std::vector<Hurtbox> &hurtboxes)
{
	BaseEntity::collectHurtboxes(hurtboxes); // the boss's own body

	// So the player can destroy the summoned processes.
	for (const std::unique_ptr<BaseEnemy> &process : summonedProcesses)
		process->collectHurtboxes(hurtboxes);

	if (forkedClone != nullptr)
		forkedClone->collectHurtboxes(hurtboxes);
}

void SegfaultBoss::drainEndedSourceIds(std::vector<std::uint32_t> &out)
{
	out.insert(out.end(), endedSourceIds.begin(), endedSourceIds.end());
	endedSourceIds.clear();

	for (std::unique_ptr<BaseEnemy> &process : summonedProcesses)
		process->drainEndedSourceIds(out);

	if (forkedClone != nullptr)
		forkedClone->drainEndedSourceIds(out);
}

json SegfaultBoss::serialize() const
{
	json j = BaseEnemy::serialize();
	j["type"] = "SegfaultBoss";
	j["stage"] = stage;
	j["stage2Triggered"] = stage2Triggered;
	j["stage3Triggered"] = stage3Triggered;
	return j;
}

void SegfaultBoss::deserialize(const json &j)
{
	BaseEnemy::deserialize(j);

	if (j.contains("stage"))
		stage = j["stage"];
	if (j.contains("stage2Triggered"))
		stage2Triggered = j["stage2Triggered"];
	if (j.contains("stage3Triggered"))
		stage3Triggered = j["stage3Triggered"];
}
