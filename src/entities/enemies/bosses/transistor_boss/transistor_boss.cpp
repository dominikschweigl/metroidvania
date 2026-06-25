#include "transistor_boss.h"
#include "../../../../core/audio_manager.h"
#include "../../../../items/usb_key_item.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>

TransistorBoss::TransistorBoss(sf::Vector2f spawnPos, float drop_chance = DROP_CHANCE)
    : BaseEnemy(spawnPos, ENTITY_WIDTH, ENTITY_HEIGHT, BOSS_HEALTH, drop_chance)
{
	currentState = &states.roaming;
}

void TransistorBoss::draw(sf::RenderWindow &window)
{
	const sf::Vector2f bodyCenter{position.x, position.y - height / 2.f};

	for (const std::unique_ptr<Capacitor> &cap : bondedCapacitors)
		renderer.drawBeam(window, bodyCenter, cap->getCenter(), auraTimer);

	if (auraPhase != AuraPhase::None) {
		using AuraStyle = transistor_boss::TransistorBossRenderer::AuraStyle;
		const AuraStyle style = (auraPhase == AuraPhase::Windup) ? AuraStyle::Windup : AuraStyle::Damage;
		renderer.drawAura(window, bodyCenter, style, auraTimer, CHARGE_AURA_RADIUS);
	}

	const Direction facing = dying ? deathFacing : direction;
	const float scaleX = (facing == Direction::Right) ? 1.f : -1.f;
	const sf::Color tint = dying              ? sf::Color::White
	                       : isHurtFlashing() ? sf::Color{255, 80, 80}
	                       : invincible       ? sf::Color{150, 220, 255}
	                                          : sf::Color::White;
	renderer.drawSprite(window, position, scaleX, tint);

	for (const projectiles::ElectricBall &ball : electricBalls)
		ball.draw(window);

	for (const std::unique_ptr<Capacitor> &cap : bondedCapacitors)
		cap->draw(window);
}

void TransistorBoss::onHit(const Hitbox & /*hit*/) noexcept
{
	triggerHurtFlash();
}

void TransistorBoss::onPreUpdate(float deltaTime)
{
	// Defeat: drop every active threat, and switch to death state.
	if (!dying && health.current <= 0) {
		dying = true;
		deathFacing = direction;

		for (const projectiles::ElectricBall &ball : electricBalls)
			endedBallSourceIds.push_back(ball.getSourceId());
		electricBalls.clear();

		for (std::unique_ptr<Capacitor> &cap : bondedCapacitors)
			cap->drainEndedSourceIds(endedBallSourceIds);
		bondedCapacitors.clear();

		if (beamSourceId != 0) {
			endedBallSourceIds.push_back(beamSourceId);
			beamSourceId = 0;
		}

		currentState->onExit(*this);
		currentState = &states.death;
		currentState->onEnter(*this);
		return;
	}

	if (dying)
		return;

	shootAttackCooldown = std::max(0.f, shootAttackCooldown - deltaTime);
	chargeAttackCooldown = std::max(0.f, chargeAttackCooldown - deltaTime);
	auraTimer += deltaTime;

	const sf::FloatRect playerBounds = lastPlayerBounds;
	std::erase_if(electricBalls, [this, deltaTime, playerBounds](projectiles::ElectricBall &ball) {
		if (ball.hasHitPlayer()) {
			endedBallSourceIds.push_back(ball.getSourceId());
			return true;
		}
		if (ball.update(deltaTime)) {
			endedBallSourceIds.push_back(ball.getSourceId());
			return true;
		}
		if (ball.getBounds().findIntersection(playerBounds).has_value())
			ball.markHitPlayer();
		return false;
	});

	if (!bondedCapacitors.empty()) {
		for (std::unique_ptr<Capacitor> &cap : bondedCapacitors)
			cap->update(deltaTime, *lastWorld, lastPlayerPos, lastPlayerBounds);

		std::erase_if(bondedCapacitors, [this](const std::unique_ptr<Capacitor> &cap) {
			if (!cap->isAlive()) {
				cap->drainEndedSourceIds(endedBallSourceIds);
				return true;
			}
			return false;
		});
	}

	if (!bondedCapacitors.empty()) {
		beamTickTimer += deltaTime;
		if (beamSourceId == 0 || beamTickTimer >= BEAM_TICK) {
			if (beamSourceId != 0)
				endedBallSourceIds.push_back(beamSourceId);
			beamSourceId = nextSourceId();
			beamTickTimer = 0.f;
		}
	} else if (beamSourceId != 0) {
		endedBallSourceIds.push_back(beamSourceId);
		beamSourceId = 0;
	}
}

std::optional<Hitbox> TransistorBoss::getHitbox() noexcept
{
	if (auraPhase != AuraPhase::Damage)
		return std::nullopt;

	const sf::Vector2f center{position.x, position.y - height / 2.f};
	const sf::FloatRect bounds({center.x - CHARGE_AURA_RADIUS, center.y - CHARGE_AURA_RADIUS},
	                           {2.f * CHARGE_AURA_RADIUS, 2.f * CHARGE_AURA_RADIUS});
	return Hitbox{bounds, CHARGE_DAMAGE, Team::Enemy, chargeSourceId};
}

void TransistorBoss::spawnElectricBall(const sf::Vector2f targetPos)
{
	const sf::Vector2f origin{position.x, position.y - height / 2.f};
	sf::Vector2f aim = targetPos - origin;
	const float length = std::hypot(aim.x, aim.y);
	if (length > 0.0001f)
		aim /= length;
	else
		aim = {(direction == Direction::Right) ? 1.f : -1.f, 0.f};

	electricBalls.emplace_back(origin, aim);
	AudioManager::getInstance().playSound(SoundEffect::TRANSISTOR_BOSS_SHOOT_ATTACK);
}

void TransistorBoss::spawnBondedCapacitors()
{
	stage2Triggered = true;
	const sf::Vector2f core{position.x, position.y - height / 2.f};
	const std::array<sf::Vector2f, CAPACITOR_COUNT> offsets = {sf::Vector2f{-90.f, -70.f}, sf::Vector2f{0.f, -130.f},
	                                                           sf::Vector2f{90.f, -70.f}};
	for (const sf::Vector2f offset : offsets)
		bondedCapacitors.push_back(std::make_unique<Capacitor>(core + offset, BaseEnemy::DROP_CHANCE));
}

void TransistorBoss::collectHitboxes(std::vector<Hitbox> &hitboxes)
{
	BaseEnemy::collectHitboxes(hitboxes);
	for (const projectiles::ElectricBall &ball : electricBalls)
		hitboxes.push_back(ball.getHitbox());
	for (const std::unique_ptr<Capacitor> &cap : bondedCapacitors)
		cap->collectHitboxes(hitboxes);
	collectBeamHitboxes(hitboxes);
}

void TransistorBoss::collectHurtboxes(std::vector<Hurtbox> &hurtboxes)
{
	BaseEntity::collectHurtboxes(hurtboxes); // the boss's own body
	for (const std::unique_ptr<Capacitor> &cap : bondedCapacitors)
		cap->collectHurtboxes(hurtboxes); // so the player can destroy the minions
}

void TransistorBoss::collectBeamHitboxes(std::vector<Hitbox> &hitboxes) const
{
	if (beamSourceId == 0)
		return;

	const sf::Vector2f core{position.x, position.y - height / 2.f};
	for (const std::unique_ptr<Capacitor> &cap : bondedCapacitors) {
		const sf::Vector2f tip = cap->getCenter();
		for (int sample = 0; sample <= BEAM_HITBOX_SAMPLES; ++sample) {
			const float t = static_cast<float>(sample) / BEAM_HITBOX_SAMPLES;
			const sf::Vector2f point = core + (tip - core) * t;
			const sf::FloatRect box({point.x - BEAM_HITBOX_SIZE / 2.f, point.y - BEAM_HITBOX_SIZE / 2.f},
			                        {BEAM_HITBOX_SIZE, BEAM_HITBOX_SIZE});
			hitboxes.push_back(Hitbox{box, BEAM_DAMAGE, Team::Enemy, beamSourceId});
		}
	}
}

void TransistorBoss::drainEndedSourceIds(std::vector<std::uint32_t> &out)
{
	out.insert(out.end(), endedBallSourceIds.begin(), endedBallSourceIds.end());
	endedBallSourceIds.clear();
	for (std::unique_ptr<Capacitor> &cap : bondedCapacitors)
		cap->drainEndedSourceIds(out);
}

void TransistorBoss::setAnimation(TransistorBossAnimation anim, int frame)
{
	renderer.setAnimation(anim, frame);
}

std::vector<std::unique_ptr<Item>> TransistorBoss::rollDrops()
{
	lootDropped = true;
	std::cout << "rollDrops" << std::endl;
	std::vector<std::unique_ptr<Item>> drops;
	drops.push_back(std::make_unique<UsbKeyItem>());
	return drops;
}

json TransistorBoss::serialize() const
{
	json j = BaseEnemy::serialize();

	j["type"] = "TransistorBoss";
	j["state"] = stage2Triggered;

	return j;
}

void TransistorBoss::deserialize(const json &j)
{
	BaseEnemy::deserialize(j);

	if (j.contains("state")) {
		stage2Triggered = j["state"];
	}
}
