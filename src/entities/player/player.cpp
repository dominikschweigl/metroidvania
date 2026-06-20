#include "player.h"
#include "../../core/input_manager.h"
#include "../entity_physics.h"
#include <algorithm>
#include <random>
#include <vector>

namespace {
std::mt19937 rng{std::random_device{}()};
} // namespace

namespace {
constexpr sf::Vector2f PLAYER_SPAWN{15 * 32.f, 0.f};
} // namespace

Inventory &Player::inventory() noexcept
{
	return inventory_;
}
const Inventory &Player::inventory() const noexcept
{
	return inventory_;
}

void Player::useHotbarSlot(const int slot)
{
	inventory_.interact({SlotKind::Hotbar, slot}, *this);
}

Player::Player()
    : BaseEntity(PLAYER_SPAWN, static_cast<float>(FRAME_SIZE), static_cast<float>(FRAME_SIZE), MAX_HEALTH,
                 Team::Player),
      lowerBodySprite(states.idle.idle_lower_texture), headSprite(AssetManager::getInstance().getTexture(PLAYER_HEAD)),
      upperBodySprite(states.idle.idle_upper_texture), currentState(&states.idle)
{
	setDirection(Direction::Left);
	setOnGround(true);
	lowerBodySprite.setOrigin({FRAME_SIZE / 2.f, static_cast<float>(FRAME_SIZE)});
	headSprite.setOrigin({FRAME_SIZE / 2.f, static_cast<float>(FRAME_SIZE)});
	upperBodySprite.setOrigin({FRAME_SIZE / 2.f, static_cast<float>(FRAME_SIZE)});
	lowerBodySprite.setPosition(position);
}

void Player::update(float deltaTime, const World &world, bool attackTriggered, bool hatThrowTriggered)
{

	tickHurtTimers(deltaTime);

	// iframes put initial invincibility on player for better combat feel
	iframes = std::max(0.f, iframes - deltaTime);
	if (health.current < previousHealth)
		iframes = IFRAME_DURATION;
	previousHealth = health.current;

	std::erase_if(activeEffects_, [deltaTime](Effect &effect) {
		effect.remainingDuration -= deltaTime;
		return effect.remainingDuration <= 0.f;
	});

	// Apply any status effect queued by onHit() (kept out of onHit to stay noexcept).
	if (pendingSlow_) {
		addEffect(Effect::slow());
		pendingSlow_ = false;
	}

	handleMovement(deltaTime, world);

	PlayerState *next = currentState->update(deltaTime, *this);
	if (next != currentState)
		transitionTo(*next);

	if (attackTriggered && currentState->canAttack() && !hatAbility.isThrowActive())
		meleeAttack.trigger();

	if (hatThrowTriggered && currentState->canAttack() && inventory_.hasHat() && hatAbility.canThrow()
	    && !isAttackActive())
		hatAbility.trigger();

	meleeAttack.update(deltaTime);

	constexpr float HEAD_Y_ORIGIN_OFFSET = -(FRAME_SIZE - 4.f);
	const sf::Vector2f headPos = position + sf::Vector2f{0.f, HEAD_Y_ORIGIN_OFFSET};

	constexpr float HAT_SPAWN_X_OFFSET = FRAME_SIZE / 2.f + 5.f;
	constexpr float HAT_SPAWN_Y_OFFSET = -FRAME_SIZE / 2.f;
	const sf::Vector2f hat_spawn_offset =
	    sf::Vector2f{static_cast<float>(getDirection()) * HAT_SPAWN_X_OFFSET, HAT_SPAWN_Y_OFFSET};
	const sf::Vector2f spawnPos = position + hat_spawn_offset;

	hatAbility.update(deltaTime, headPos, spawnPos, getDirection(), velocity, world);

	updateAnimation(deltaTime);
}

std::optional<Hitbox> Player::getMeleeHitbox() const noexcept
{
	std::optional<Hitbox> hitbox = meleeAttack.getHitbox(position, getDirection());
	if (hitbox)
		hitbox->damage = static_cast<int>(static_cast<float>(hitbox->damage) * damageMultiplier());
	return hitbox;
}

void Player::collectHitboxes(std::vector<Hitbox> &hitboxes)
{
	if (const std::optional<Hitbox> melee = getMeleeHitbox())
		hitboxes.push_back(*melee);
	if (hasHatThrown())
		hitboxes.push_back(getThrownHat().getHitbox());
}

void Player::addEffect(Effect effect)
{
	for (Effect &existing : activeEffects_) {
		if (existing.effectId() == effect.effectId()) {
			existing.remainingDuration = effect.totalDuration;
			return;
		}
	}
	activeEffects_.push_back(effect);
}

const std::vector<Effect> &Player::activeEffects() const noexcept
{
	return activeEffects_;
}

float Player::speedMultiplier() const noexcept
{
	float result = 1.f;
	for (const Effect &effect : activeEffects_)
		result *= effect.speedMultiplier();
	return result;
}

float Player::jumpMultiplier() const noexcept
{
	float result = 1.f;
	for (const Effect &effect : activeEffects_)
		result *= effect.jumpMultiplier();
	return result;
}

float Player::damageMultiplier() const noexcept
{
	float result = 1.f;
	for (const Effect &effect : activeEffects_)
		result *= effect.damageMultiplier();
	return result;
}

float Player::damageResistance() const noexcept
{
	float result = 0.f;
	for (const Effect &effect : activeEffects_)
		result = std::max(result, effect.damageResistance());
	return result;
}

void Player::takeDamage(const int amount) noexcept
{
	const float resistance = damageResistance();
	if (resistance <= 0.f) {
		health.damage(amount);
		return;
	}
	const float reducedDamage = static_cast<float>(amount) * (1.f - resistance);
	const int guaranteedDamage = static_cast<int>(reducedDamage);
	const float fraction = reducedDamage - static_cast<float>(guaranteedDamage);
	std::uniform_real_distribution<float> dist{0.f, 1.f};
	const int extraDamage = (fraction > 0.f && dist(rng) < fraction) ? 1 : 0;
	health.damage(guaranteedDamage + extraDamage);
}

void Player::onHit(const Hitbox &hit) noexcept
{
	BaseEntity::onHit(hit);
	if (hit.statusOnHit == StatusEffectKind::Slow)
		pendingSlow_ = true;
}

void Player::drainEndedSourceIds(std::vector<std::uint32_t> &out)
{
	meleeAttack.drainEndedSourceIds(out);
	hatAbility.drainEndedSourceIds(out);
}

void Player::updateAnimation(float dt)
{
	const float facingMultiplier = static_cast<float>(getDirection());
	const sf::Vector2f scale{facingMultiplier, 1.f};

	const sf::Color hurtTint{255, 80, 80};
	const sf::Color spriteColor = isHurtFlashing() ? hurtTint : sf::Color::White;
	lowerBodySprite.setColor(spriteColor);
	upperBodySprite.setColor(spriteColor);
	headSprite.setColor(spriteColor);

	currentState->applyAnimation(dt, *this);
	lowerBodySprite.setPosition(position);
	lowerBodySprite.setScale(scale);
	upperBodySprite.setScale(scale);

	const sf::Vector2f upperOffset = currentState->getUpperBodyOffset(*this);
	upperBodySprite.setPosition(position + sf::Vector2f{upperOffset.x * scale.x, upperOffset.y});

	const bool hatOnHead = inventory_.hasHat() && hatAbility.isHatOnHead();
	headSprite.setTexture(AssetManager::getInstance().getTexture(hatOnHead ? PLAYER_HEAD_HAT : PLAYER_HEAD));
	headSprite.setTextureRect(sf::IntRect({0, 0}, {FRAME_SIZE, FRAME_SIZE}));
	const sf::Vector2f headOffset = currentState->getHeadOffset(*this);
	headSprite.setPosition(position + sf::Vector2f{headOffset.x * scale.x, headOffset.y});
	headSprite.setScale(scale);

	if (isAttackActive() && currentState->canAttack()) {
		if (meleeAttack.isMeleeActive()) {
			meleeAttack.applyAnimation(upperBodySprite, scale, upperBodySprite.getPosition());
		} else if (hatAbility.isThrowActive()) {
			hatAbility.applyAnimation(upperBodySprite, scale, upperBodySprite.getPosition());
		}
	}
}

void Player::handleMovement(float deltaTime, const World &world)
{
	InputManager &input = InputManager::getInstance();
	inputJump = input.isHeld(GameAction::Jump);
	inputLeft = input.isHeld(GameAction::MoveLeft);
	inputRight = input.isHeld(GameAction::MoveRight);

	// Knockback temporarily overrides velocity.
	if (!isKnockedBack()) {
		if (wallJumpTimer > 0.f) {
			wallJumpTimer -= deltaTime;
		} else {
			velocity.x = 0.f;
			isSprinting = input.isHeld(GameAction::Sprint);
			const float speed = (isSprinting ? RUNNING_SPEED : WALKING_SPEED) * speedMultiplier();

			if (inputLeft) {
				velocity.x = -speed;
				setDirection(Direction::Left);
			}
			if (inputRight) {
				velocity.x = speed;
				setDirection(Direction::Right);
			}
		}
	}

	EntityPhysics::simulateMovement(deltaTime, position, velocity, isOnGround, gravity, width, height, world);
	isAgainstLeftWall = EntityPhysics::isWallOnLeft(position, width, height, world);
	isAgainstRightWall = EntityPhysics::isWallOnRight(position, width, height, world);
}

void Player::transitionTo(PlayerState &next)
{
	currentState->onExit(*this);
	next.onEnter(*this);
	currentState = &next;
	if (!currentState->canAttack()) {
		meleeAttack.reset();
		hatAbility.reset();
	}
}

void Player::draw(sf::RenderWindow &window)
{
	if (debugHorizontalMovement)
		window.draw(debugHorizontalCollisionCheck);
	if (debugVerticalMovement)
		window.draw(debugVerticalCollisionCheck);
	window.draw(lowerBodySprite);
	window.draw(headSprite);
	window.draw(upperBodySprite);
	hatAbility.draw(window);
}

json Player::serialize() const
{
	json j = BaseEntity::serialize();

	j["inventory"] = inventory_.serialize();

	return j;
}
void Player::deserialize(const json &j)
{
	BaseEntity::deserialize(j);

	if (j.contains("inventory")) {
		inventory_.deserialize(j["inventory"]);
	}
}
