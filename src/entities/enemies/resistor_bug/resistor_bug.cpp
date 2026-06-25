#include "resistor_bug.h"
#include "../../../core/audio_manager.h"
#include <algorithm>

ResistorBug::ResistorBug(sf::Vector2f spawnPos, float drop_chance = DROP_CHANCE)
    : BaseEnemy(spawnPos, ENTITY_WIDTH, ENTITY_HEIGHT, MAX_HEALTH, drop_chance),
      idleTexture(AssetManager::getInstance().getTexture(RESISTOR_BUG_IDLE)),
      movingTexture(AssetManager::getInstance().getTexture(RESISTOR_BUG_MOVING)),
      telegraphTexture(AssetManager::getInstance().getTexture(RESISTOR_BUG_IDLE)),
      attackTexture(AssetManager::getInstance().getTexture(RESISTOR_BUG_IDLE)),
      recoverTexture(AssetManager::getInstance().getTexture(RESISTOR_BUG_IDLE)), sprite(idleTexture)
{
	sprite.setOrigin({FRAME_SIZE / 2.f, static_cast<float>(FRAME_SIZE)});
	currentState = &states.idle;
}

void ResistorBug::draw(sf::RenderWindow &window)
{
	sprite.setPosition(position);
	sprite.setScale({direction == Direction::Right ? 1.f : -1.f, 1.f});
	sprite.setColor(isHurtFlashing() ? sf::Color{255, 80, 80} : sf::Color::White);
	window.draw(sprite);
}

void ResistorBug::onPreUpdate(float deltaTime)
{
	attackCooldown = std::max(0.f, attackCooldown - deltaTime);
	jumpCooldown = std::max(0.f, jumpCooldown - deltaTime);
}

void ResistorBug::launchHop(sf::Vector2f playerPos)
{
	const float sign = (playerPos.x >= position.x) ? 1.f : -1.f;
	velocity.x = sign * HOP_VX;
	velocity.y = -HOP_VY;
	isOnGround = false;
	jumpCooldown = JUMP_COOLDOWN;

	attackActive = true;
	attackSourceId = nextSourceId();
	AudioManager::getInstance().playSound(SoundEffect::SLIME_JUMP);
}

void ResistorBug::endAttack() noexcept
{
	if (!attackActive)
		return;
	attackActive = false;
	endedSourceIds.push_back(attackSourceId);
}

void ResistorBug::drainEndedSourceIds(std::vector<std::uint32_t> &out)
{
	out.insert(out.end(), endedSourceIds.begin(), endedSourceIds.end());
	endedSourceIds.clear();
}

void ResistorBug::setAnimation(BugAnimation anim, int frame)
{
	switch (anim) {
	case BugAnimation::Idle:
		sprite.setTexture(idleTexture);
		break;
	case BugAnimation::Moving:
		sprite.setTexture(movingTexture);
		break;
	case BugAnimation::Telegraph:
		sprite.setTexture(telegraphTexture);
		break;
	case BugAnimation::Attack:
		sprite.setTexture(attackTexture);
		break;
	case BugAnimation::Recover:
		sprite.setTexture(recoverTexture);
		break;
	}
	sprite.setTextureRect(sf::IntRect({frame * FRAME_SIZE, 0}, {FRAME_SIZE, FRAME_SIZE}));
}

json ResistorBug::serialize() const
{
	json j = BaseEnemy::serialize();

	j["type"] = "ResistorBug";

	return j;
}
