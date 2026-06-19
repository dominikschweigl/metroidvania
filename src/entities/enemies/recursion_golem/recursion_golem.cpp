#include "recursion_golem.h"
#include "../../../core/asset_manager.h"
#include "../../../core/audio_manager.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

float RecursionGolem::widthForSize(int size) noexcept
{
	return BASE_WIDTH + static_cast<float>(size) * WIDTH_PER_SIZE;
}

float RecursionGolem::heightForSize(int size) noexcept
{
	// Golems are roughly square; keep height in step with width.
	return BASE_WIDTH + static_cast<float>(size) * WIDTH_PER_SIZE;
}

RecursionGolem::RecursionGolem(sf::Vector2f spawnPos, int size)
    : BaseEnemy(spawnPos, widthForSize(size), heightForSize(size), std::max(1, size)), size_(size),
      idleTexture(AssetManager::getInstance().getTexture(GOLEM_IDLE)),
      movingTexture(AssetManager::getInstance().getTexture(GOLEM_MOVING)),
      windupTexture(AssetManager::getInstance().getTexture(GOLEM_WIND_UP)),
      attackTexture(AssetManager::getInstance().getTexture(GOLEM_ATTACK)),
      explodeTexture(AssetManager::getInstance().getTexture(GOLEM_EXPLODE)), sprite(idleTexture),
      rng(std::random_device{}())
{
	sprite.setOrigin({FRAME_SIZE / 2.f, static_cast<float>(FRAME_SIZE)});
	currentState = &states.idle;
}

float RecursionGolem::moveSpeed() const noexcept
{
	// Smaller golems are faster: each step below MAX_SIZE adds a speed bonus.
	const int shrink = std::max(0, MAX_SIZE - size_);
	return BASE_MOVE_SPEED * (1.f + 0.18f * static_cast<float>(shrink));
}

void RecursionGolem::draw(sf::RenderWindow &window)
{
	float scale = widthForSize(size_) / static_cast<float>(FRAME_SIZE);
	float bob = 0.f;

	if (isExploding_) {
		// Swell and pulse brighter the closer the stack-overflow blast gets.
		const float progress = std::clamp(1.f - explodeTimer_ / EXPLODE_COUNTDOWN, 0.f, 1.f);
		scale *= 1.f + EXPLODE_SWELL * progress;
		const std::uint8_t pulse = static_cast<std::uint8_t>(120 + 135 * std::abs(std::sin(animClock_ * 18.f)));
		sprite.setColor(sf::Color{255, pulse, 60});
	} else {
		if (isOnGround)
			bob = std::sin(animClock_ * BOB_SPEED) * BOB_AMPLITUDE;
		sprite.setColor(isHurtFlashing() ? sf::Color{255, 80, 80} : sf::Color::White);
	}

	sprite.setPosition(position + sf::Vector2f{0.f, bob});
	sprite.setScale({direction == Direction::Right ? scale : -scale, scale});

	window.draw(sprite);
}

void RecursionGolem::onPreUpdate(float deltaTime)
{
	attackCooldown = std::max(0.f, attackCooldown - deltaTime);
	animClock_ += deltaTime;

	if (defeated_ && !resolved_)
		resolveDefeat();

	if (isExploding_) {
		if (!explosionFired_) {
			explodeTimer_ -= deltaTime;
			if (explodeTimer_ <= 0.f) {
				explosionActive_ = true;
				explosionFired_ = true;
				explosionSourceId = nextSourceId();
				AudioManager::getInstance().playSound(SoundEffect::TRANSISTOR_BOSS_EXPLOSION);
			}
		} else if (explosionActive_) {
			explosionActive_ = false;
			endedSourceIds.push_back(explosionSourceId);
			removeRequested_ = true;
		}
	}
}

void RecursionGolem::resolveDefeat()
{
	resolved_ = true;

	if (isBaseCase()) {
		// Stack Overflow: begin the explosion countdown instead of dying outright.
		isExploding_ = true;
		explodeTimer_ = EXPLODE_COUNTDOWN;
		currentState = &states.explode;
		states.explode.onEnter(*this);
		return;
	}

	// Recursive decomposition: fib(n) -> fib(n-1) + fib(n-2).
	AudioManager::getInstance().playSound(SoundEffect::SLIME_JUMP);
	const float offset = widthForSize(size_) * 0.5f;
	spawnChild(size_ - 1, -offset);
	spawnChild(size_ - 2, offset);
	removeRequested_ = true;
}

void RecursionGolem::spawnChild(int childSize, float offsetX)
{
	auto child = std::make_unique<RecursionGolem>(sf::Vector2f{position.x + offsetX, position.y}, childSize);
	child->setVelocity({offsetX >= 0.f ? SPLIT_POP_X : -SPLIT_POP_X, -SPLIT_POP_Y});
	child->setOnGround(false);
	pendingSpawns_.push_back(std::move(child));
}

std::optional<Hitbox> RecursionGolem::getHitbox() noexcept
{
	if (explosionActive_) {
		const float radius = widthForSize(size_) * EXPLODE_RADIUS_FACTOR;
		const sf::Vector2f center{position.x, position.y - height / 2.f};
		const sf::FloatRect area({center.x - radius, center.y - radius}, {radius * 2.f, radius * 2.f});
		Hitbox hit{area, EXPLODE_DAMAGE, Team::Enemy, explosionSourceId};
		hit.statusOnHit = StatusEffectKind::Slow;
		return hit;
	}

	if (isAttacking()) {
		Hitbox hit{getBounds(), ATTACK_DAMAGE, Team::Enemy, attackSourceId};
		hit.statusOnHit = StatusEffectKind::Slow;
		return hit;
	}

	return std::nullopt;
}

void RecursionGolem::takeDamage(int amount) noexcept
{
	health.damage(amount);
	if (!health.isAlive())
		defeated_ = true;
}

bool RecursionGolem::isAlive() const noexcept
{
	if (removeRequested_)
		return false;
	if (isExploding_)
		return true; // survive through the countdown so the blast can land
	return health.isAlive();
}

void RecursionGolem::drainSpawns(std::vector<std::unique_ptr<BaseEnemy>> &out)
{
	for (auto &child : pendingSpawns_)
		out.push_back(std::move(child));
	pendingSpawns_.clear();
}

void RecursionGolem::drainEndedSourceIds(std::vector<std::uint32_t> &out)
{
	out.insert(out.end(), endedSourceIds.begin(), endedSourceIds.end());
	endedSourceIds.clear();
}

void RecursionGolem::setAnimation(GolemAnimation anim, int frame)
{
	switch (anim) {
	case GolemAnimation::Idle:
		sprite.setTexture(idleTexture);
		break;
	case GolemAnimation::Moving:
		sprite.setTexture(movingTexture);
		break;
	case GolemAnimation::WindUp:
		sprite.setTexture(windupTexture);
		break;
	case GolemAnimation::Attack:
		sprite.setTexture(attackTexture);
		break;
	case GolemAnimation::Explode:
		sprite.setTexture(explodeTexture);
		break;
	}
	sprite.setTextureRect(sf::IntRect({frame * FRAME_SIZE, 0}, {FRAME_SIZE, FRAME_SIZE}));
}

json RecursionGolem::serialize() const
{
	json j = BaseEnemy::serialize();
	j["type"] = "RecursionGolem";
	j["size"] = size_;
	return j;
}
