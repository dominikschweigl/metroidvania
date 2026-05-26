#include "hat_ability.h"
#include "../../core/audio_manager.h"
#include <cassert>

HatAbility::HatAbility()
    : throwTexture(AssetManager::getInstance().getTexture(PLAYER_ATTACK_THROW_HAT)),
      projectileTexture(AssetManager::getInstance().getTexture(PLAYER_HAT_PROJECTILE))
{
}

bool HatAbility::canThrow() const noexcept
{
	return !throwActive && !projectile.has_value();
}

bool HatAbility::isThrowActive() const noexcept
{
	return throwActive;
}

bool HatAbility::isHatOnHead() const noexcept
{
	return !projectile.has_value() && (!throwActive || throwFrame < HAT_DETACH_FRAME);
}

bool HatAbility::hasProjectile() const noexcept
{
	return projectile.has_value();
}

HatProjectile &HatAbility::getProjectile() noexcept
{
	assert(projectile.has_value());
	return *projectile;
}

void HatAbility::trigger()
{
	AudioManager::getInstance().playSound(SoundEffect::PLAYER_HAT_THROW);
	throwActive = true;
	throwFrame = 0;
	throwTimer = 0.f;
}

void HatAbility::update(float dt, sf::Vector2f headPos, sf::Vector2f spawnPos, Direction direction,
                        sf::Vector2f playerVelocity, const World &world)
{
	if (throwActive) {
		throwTimer += dt;
		if (throwTimer >= THROW_FRAME_DUR) {
			throwTimer -= THROW_FRAME_DUR;
			if (++throwFrame >= THROW_FRAME_COUNT) {
				throwActive = false;
				throwFrame = 0;
				projectile.emplace(spawnPos, direction, playerVelocity, projectileTexture);
			}
		}
	}

	if (projectile.has_value()) {
		bool isReturnedToHead = projectile->update(dt, headPos, world);
		if (isReturnedToHead) {
			projectile.reset();
		}
	}
}

void HatAbility::applyAnimation(sf::Sprite &upper, sf::Vector2f scale, sf::Vector2f pos) const
{
	upper.setTexture(throwTexture);
	upper.setTextureRect(sf::IntRect({throwFrame * 32, 0}, {32, 32}));
	upper.setPosition(pos);
	upper.setScale(scale);
}

void HatAbility::draw(sf::RenderWindow &window) const
{
	if (projectile.has_value())
		projectile->draw(window);
}

void HatAbility::reset() noexcept
{
	throwActive = false;
	throwFrame = 0;
	throwTimer = 0.f;
}
