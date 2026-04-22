#include "race_condition_slime.h"
#include <algorithm>
#include <cmath>

RaceConditionSlime::RaceConditionSlime(sf::Vector2f spawnPos)
    : BaseEnemy(spawnPos, ENTITY_WIDTH, ENTITY_HEIGHT), rng(std::random_device{}())
{
	loadAssets();
	sprite.setOrigin({FRAME_SIZE / 2.f, static_cast<float>(FRAME_SIZE)});
	resetTeleportTimer();
	currentState = &states.idle;
}

void RaceConditionSlime::draw(sf::RenderWindow &window)
{
	sprite.setPosition(pos);
	sprite.setScale({facing == Direction::Right ? 1.f : -1.f, 1.f});
	window.draw(sprite);
}

void RaceConditionSlime::onPreUpdate(float deltaTime)
{
	attackCooldown = std::max(0.f, attackCooldown - deltaTime);
	jumpCooldown = std::max(0.f, jumpCooldown - deltaTime);
	teleportTimer -= deltaTime;
}

void RaceConditionSlime::tryJumpTowards(float heightDiff)
{
	if (!isOnGround || heightDiff <= JUMP_THRESHOLD || jumpCooldown > 0.f) {
		return;
	}
	// Jump velocity: v = sqrt(2 * g * h), capped at MAX_JUMP_SPEED.
	float necessaryVelocity = std::sqrt(2.f * gravity * (heightDiff + ENTITY_HEIGHT));
	vel.y = -std::min(necessaryVelocity, MAX_JUMP_SPEED);
	isOnGround = false;
	jumpCooldown = JUMP_COOLDOWN;
}

void RaceConditionSlime::maybeTeleport(const World &world, sf::Vector2f playerPos)
{
	if (teleportTimer <= 0.f) {
		glitchTeleport(world, playerPos);
	}
}

void RaceConditionSlime::setAnimation(SlimeAnimation anim, int frame)
{
	const sf::Texture *tex = nullptr;
	switch (anim) {
	case SlimeAnimation::Idle:
		tex = &idleTexture;
		break;
	case SlimeAnimation::Moving:
		tex = &movingTexture;
		break;
	case SlimeAnimation::WindUp:
		tex = &windupTexture;
		break;
	case SlimeAnimation::Attack:
		tex = &attackTexture;
		break;
	case SlimeAnimation::Recover:
		tex = &recoverTexture;
		break;
	}
	sprite.setTexture(*tex);
	sprite.setTextureRect(sf::IntRect({frame * FRAME_SIZE, 0}, {FRAME_SIZE, FRAME_SIZE}));
}

void RaceConditionSlime::resetTeleportTimer()
{
	teleportTimer = uniformFloat(1.0f, 2.5f);
}

void RaceConditionSlime::glitchTeleport(const World &world, sf::Vector2f playerPos)
{
	resetTeleportTimer();
	float dir = (playerPos.x >= pos.x) ? 1.f : -1.f;

	// Up to 8 attempts to find a landing spot that is neither inside a wall
	// nor over a pit too deep to jump back out of.
	for (int attempt = 0; attempt < 8; ++attempt) {
		switch (std::uniform_int_distribution<int>(0, 2)(rng)) {
		case 0: { // forward
			float newX = pos.x + dir * uniformFloat(80.f, 220.f);
			if (isValidTeleportDest(world, newX, pos.y)) {
				pos.x = newX;
				return;
			}
			break;
		}
		case 1: { // backward
			float newX = pos.x - dir * uniformFloat(60.f, 160.f);
			if (isValidTeleportDest(world, newX, pos.y)) {
				pos.x = newX;
				return;
			}
			break;
		}
		case 2: // upward in-place velocity boost
			vel.y = -uniformFloat(380.f, 580.f);
			isOnGround = false;
			return;
		}
	}
}

bool RaceConditionSlime::isValidTeleportDest(const World &world, float newX, float newY) const
{
	sf::FloatRect dest({newX - ENTITY_WIDTH / 2.f, newY - ENTITY_HEIGHT}, {ENTITY_WIDTH, ENTITY_HEIGHT});
	if (world.isSolidAtRect(dest)) {
		return false;
	}

	constexpr float MAX_FALL = 5.f * World::TILE_SIZE; // stays within jump reach
	constexpr float STEP = World::TILE_SIZE / 2.f;
	for (int i = 1; i * STEP <= MAX_FALL; ++i) {
		float delta = i * STEP;
		float y = newY + delta;
		auto lhs = world.getTileAtCoordinate({newX - ENTITY_WIDTH / 2.f, y});
		auto rhs = world.getTileAtCoordinate({newX + ENTITY_WIDTH / 2.f, y});
		if ((lhs.has_value() && lhs.value()->isSolid) || (rhs.has_value() && rhs.value()->isSolid)) {
			return true;
		}
	}
	return false;
}

void RaceConditionSlime::loadAssets()
{

	// ToDo: Extract asset logic for shared sprite cache (Flighweight Pattern?)

	(void)idleTexture.loadFromFile("./assets/images/enemies/race_condition/idle.png");
	(void)movingTexture.loadFromFile("./assets/images/enemies/race_condition/moving.png");
	(void)windupTexture.loadFromFile("./assets/images/enemies/race_condition/windup.png");
	(void)attackTexture.loadFromFile("./assets/images/enemies/race_condition/attack.png");
	(void)recoverTexture.loadFromFile("./assets/images/enemies/race_condition/recover.png");
	sprite.setTexture(idleTexture);
}
