#include "capacitor.h"
#include "../../../core/asset_manager.h"
#include "../../../core/audio_manager.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

Capacitor::Capacitor(sf::Vector2f spawnPos) : Capacitor::Capacitor(spawnPos, DROP_CHANCE) {}

Capacitor::Capacitor(sf::Vector2f spawnPos, float drop_chance)
    : BaseEnemy(spawnPos, ENTITY_WIDTH, ENTITY_HEIGHT, CAPACITOR_HEALTH, drop_chance),
      hoverTexture(AssetManager::getInstance().getTexture(CAPACITOR_HOVER)), sprite(hoverTexture)
{
	gravity = 0.f; // no gravitiy for flyers
	frameCount = std::max(1, static_cast<int>(hoverTexture.getSize().x) / FRAME_SIZE);
	sprite.setOrigin({FRAME_SIZE / 2.f, FRAME_SIZE / 2.f});
	sprite.setTextureRect(sf::IntRect({0, 0}, {FRAME_SIZE, FRAME_SIZE}));
	currentState = &states.hover;
}

sf::Vector2f Capacitor::getCenter() const noexcept
{
	return {position.x, position.y - height / 2.f};
}

sf::Vector2f Capacitor::hoverTarget(const sf::Vector2f playerPos) const noexcept
{
	const float side = (position.x >= playerPos.x) ? 1.f : -1.f;
	const float targetX = playerPos.x + side * STANDOFF_X;
	// Stay reachable: never steer higher than MAX_REACH_ABOVE over the player's feet.
	const float targetY = std::max(playerPos.y - HOVER_HEIGHT, playerPos.y - MAX_REACH_ABOVE);
	return {targetX, targetY};
}

sf::Vector2f Capacitor::swoopTarget(const sf::Vector2f playerPos) const noexcept
{
	// Dive to a close, low spot beside the player, allows for attack.
	const float side = (position.x >= playerPos.x) ? 1.f : -1.f;
	return {playerPos.x + side * SWOOP_STANDOFF, playerPos.y - SWOOP_HEIGHT};
}

void Capacitor::onPreUpdate(const float deltaTime)
{
	shootCooldown = std::max(0.f, shootCooldown - deltaTime);
	bobTimer += deltaTime;

	constexpr float FRAME_DURATION = 0.09f;
	animTimer += deltaTime;
	if (animTimer >= FRAME_DURATION) {
		animTimer -= FRAME_DURATION;
		currentFrame = (currentFrame + 1) % frameCount;
	}

	const sf::FloatRect playerBounds = lastPlayerBounds;
	std::erase_if(shots, [this, deltaTime, playerBounds](projectiles::ElectricBall &shot) {
		if (shot.hasHitPlayer()) {
			endedShotSourceIds.push_back(shot.getSourceId());
			return true;
		}
		if (shot.update(deltaTime)) {
			endedShotSourceIds.push_back(shot.getSourceId());
			return true;
		}
		if (shot.getBounds().findIntersection(playerBounds).has_value())
			shot.markHitPlayer();
		return false;
	});
}

void Capacitor::spawnShot(const sf::Vector2f targetPos)
{
	const sf::Vector2f origin = getCenter();
	sf::Vector2f aim = targetPos - origin;
	const float length = std::hypot(aim.x, aim.y);
	if (length > 0.0001f)
		aim /= length;
	else
		aim = {(direction == Direction::Right) ? 1.f : -1.f, 0.f};

	shots.emplace_back(origin, aim, SHOT_SPEED, SHOT_RADIUS);
	++shotsSinceSwoop;
	AudioManager::getInstance().playSound(SoundEffect::TRANSISTOR_BOSS_SHOOT_ATTACK);
}

void Capacitor::collectHitboxes(std::vector<Hitbox> &hitboxes)
{
	BaseEnemy::collectHitboxes(hitboxes);
	for (const projectiles::ElectricBall &shot : shots)
		hitboxes.push_back(shot.getHitbox());
}

void Capacitor::drainEndedSourceIds(std::vector<std::uint32_t> &out)
{
	out.insert(out.end(), endedShotSourceIds.begin(), endedShotSourceIds.end());
	endedShotSourceIds.clear();
}

void Capacitor::draw(sf::RenderWindow &window)
{
	const float bob = std::sin(bobTimer * BOB_SPEED) * BOB_AMPLITUDE;
	sprite.setPosition(getCenter() + sf::Vector2f{0.f, bob});
	sprite.setScale({direction == Direction::Right ? 1.f : -1.f, 1.f});
	sprite.setTextureRect(sf::IntRect({currentFrame * FRAME_SIZE, 0}, {FRAME_SIZE, FRAME_SIZE}));
	const sf::Color tint = isHurtFlashing() ? sf::Color{255, 120, 120}
	                       : isSwooping()   ? sf::Color{255, 238, 170}
	                                        : sf::Color::White;
	sprite.setColor(tint);
	window.draw(sprite);

	for (const projectiles::ElectricBall &shot : shots)
		shot.draw(window);
}

json Capacitor::serialize() const
{
	json j = BaseEnemy::serialize();

	j["type"] = "Capacitor";

	return j;
}
