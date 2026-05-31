#include "race_condition_slime.h"
#include "../../../core/audio_manager.h"
#include "../../../items/healing_potion_item.h"
#include "../../../world/world.h"
#include <algorithm>
#include <cmath>
#include <random>

RaceConditionSlime::RaceConditionSlime(sf::Vector2f spawnPos)
    : BaseEnemy(spawnPos, ENTITY_WIDTH, ENTITY_HEIGHT), idleTexture(AssetManager::getInstance().getTexture(SLIME_IDLE)),
      movingTexture(AssetManager::getInstance().getTexture(SLIME_MOVING)),
      windupTexture(AssetManager::getInstance().getTexture(SLIME_WIND_UP)),
      attackTexture(AssetManager::getInstance().getTexture(SLIME_ATTACK)),
      recoverTexture(AssetManager::getInstance().getTexture(SLIME_RECOVER)), sprite(idleTexture),
      rng(std::random_device{}())
{
	sprite.setOrigin({FRAME_SIZE / 2.f, static_cast<float>(FRAME_SIZE)});
	resetTeleportTimer();
	currentState = &states.idle;
}

void RaceConditionSlime::draw(sf::RenderWindow &window)
{
	sprite.setPosition(position);
	sprite.setScale({direction == Direction::Right ? 1.f : -1.f, 1.f});
	sprite.setColor(isHurtFlashing() ? sf::Color{255, 80, 80} : sf::Color::White);
	window.draw(sprite);
}

void RaceConditionSlime::onPreUpdate(float deltaTime)
{
	attackCooldown = std::max(0.f, attackCooldown - deltaTime);
	jumpCooldown = std::max(0.f, jumpCooldown - deltaTime);
	teleportTimer -= deltaTime;

	moveSoundTimer = std::max(0.f, moveSoundTimer - deltaTime);
	if (moveSoundTimer <= 0.f && isOnGround && std::abs(velocity.x) > 0.f) {
		AudioManager::getInstance().playSound(SoundEffect::SLIME_MOVE, SLIME_VOLUME);
		moveSoundTimer = MOVE_SOUND_INTERVAL;
	}
}

void RaceConditionSlime::tryJumpTowards(float heightDiff)
{
	if (!isOnGround || heightDiff <= JUMP_THRESHOLD || jumpCooldown > 0.f) {
		return;
	}
	AudioManager::getInstance().playSound(SoundEffect::SLIME_JUMP, SLIME_VOLUME);
	// Jump velocity: v = sqrt(2 * g * h), capped at MAX_JUMP_SPEED.
	float necessaryVelocity = std::sqrt(2.f * gravity * (heightDiff + ENTITY_HEIGHT));
	velocity.y = -std::min(necessaryVelocity, MAX_JUMP_SPEED);
	isOnGround = false;
	jumpCooldown = JUMP_COOLDOWN;
	moveSoundTimer = MOVE_SOUND_INTERVAL;
}

void RaceConditionSlime::maybeTeleport(const World &world, sf::Vector2f playerPos)
{
	if (teleportTimer <= 0.f) {
		glitchTeleport(world, playerPos);
	}
}

void RaceConditionSlime::drainEndedSourceIds(std::vector<std::uint32_t> &out)
{
	out.insert(out.end(), endedSourceIds.begin(), endedSourceIds.end());
	endedSourceIds.clear();
}

void RaceConditionSlime::setAnimation(SlimeAnimation anim, int frame)
{
	switch (anim) {
	case SlimeAnimation::Idle:
		sprite.setTexture(idleTexture);
		break;
	case SlimeAnimation::Moving:
		sprite.setTexture(movingTexture);
		break;
	case SlimeAnimation::WindUp:
		sprite.setTexture(windupTexture);
		break;
	case SlimeAnimation::Attack:
		sprite.setTexture(attackTexture);
		break;
	case SlimeAnimation::Recover:
		sprite.setTexture(recoverTexture);
		break;
	}
	sprite.setTextureRect(sf::IntRect({frame * FRAME_SIZE, 0}, {FRAME_SIZE, FRAME_SIZE}));
}

void RaceConditionSlime::resetTeleportTimer()
{
	teleportTimer = uniformFloat(1.0f, 2.5f);
}

void RaceConditionSlime::glitchTeleport(const World &world, sf::Vector2f playerPos)
{
	resetTeleportTimer();
	const float dir = (playerPos.x >= position.x) ? 1.f : -1.f;

	// Up to 8 attempts to find a landing spot that is neither inside a wall
	// nor over a pit too deep to jump back out of.
	for (int attempt = 0; attempt < 8; ++attempt) {
		switch (std::uniform_int_distribution<int>(0, 2)(rng)) {
		case 0: { // forward
			const float newX = position.x + dir * uniformFloat(80.f, 220.f);
			if (isValidTeleportDest(world, newX, position.y)) {
				position.x = newX;
				return;
			}
			break;
		}
		case 1: { // backward
			const float newX = position.x - dir * uniformFloat(60.f, 160.f);
			if (isValidTeleportDest(world, newX, position.y)) {
				position.x = newX;
				return;
			}
			break;
		}
		case 2: // upward in-place velocity boost
			velocity.y = -uniformFloat(380.f, 580.f);
			isOnGround = false;
			return;
		}
	}
}

std::vector<std::unique_ptr<Item>> RaceConditionSlime::rollDrops()
{
	static constexpr float DROP_CHANCE = 0.4f;
	std::bernoulli_distribution dist(DROP_CHANCE);
	if (dist(rng)) {
		std::vector<std::unique_ptr<Item>> drops;
		drops.push_back(std::make_unique<HealingPotionItem>());
		return drops;
	}
	return {};
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
