#pragma once
#include "../../../core/asset_manager.h"
#include "../../../world/world.h"
#include "../../direction.h"
#include "hat_projectile.h"
#include <SFML/Graphics.hpp>
#include <optional>

class HatAbility {
  public:
	HatAbility();
	~HatAbility() = default;

	[[nodiscard]] bool canThrow() const noexcept;
	[[nodiscard]] bool isThrowActive() const noexcept;
	[[nodiscard]] bool isHatOnHead() const noexcept;
	[[nodiscard]] bool hasProjectile() const noexcept;
	[[nodiscard]] HatProjectile &getProjectile() noexcept;

	void trigger();
	void update(float dt, sf::Vector2f headPos, sf::Vector2f spawnPos, Direction direction, sf::Vector2f playerVelocity,
	            const World &world);
	void applyAnimation(sf::Sprite &upper, sf::Vector2f scale, sf::Vector2f pos) const;
	void draw(sf::RenderWindow &window) const;
	void reset() noexcept;

  private:
	static constexpr int THROW_FRAME_COUNT = 8;
	static constexpr float THROW_FRAME_DUR = 0.06f;
	static constexpr int HAT_DETACH_FRAME = 3;

	const sf::Texture &throwTexture;
	const sf::Texture &projectileTexture;

	bool throwActive = false;
	int throwFrame = 0;
	float throwTimer = 0.f;

	std::optional<HatProjectile> projectile;
};
