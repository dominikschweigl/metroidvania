#pragma once

#include <SFML/Graphics.hpp>

namespace segfault_boss {

// Owns the boss's sprite, its sprite-sheet textures, and all generated visuals
class SegfaultBossRenderer {
  public:
	enum class Animation { Idle, Roaming, Attack, Death };

	static constexpr int FRAME_SIZE = 64;
	static constexpr float SPRITE_SCALE = 1.f;

	SegfaultBossRenderer();
	~SegfaultBossRenderer() = default;
	SegfaultBossRenderer(const SegfaultBossRenderer &) = delete;
	SegfaultBossRenderer &operator=(const SegfaultBossRenderer &) = delete;
	SegfaultBossRenderer(SegfaultBossRenderer &&) = delete;
	SegfaultBossRenderer &operator=(SegfaultBossRenderer &&) = delete;

	void setAnimation(Animation anim, int frame);

	void drawSprite(sf::RenderWindow &window, sf::Vector2f position, float scaleX, sf::Color tint);

	void drawSpearTelegraph(sf::RenderWindow &window, sf::Vector2f footPos, float width, float timer) const;
	void drawSpear(sf::RenderWindow &window, sf::Vector2f footPos, float width, float height, float timer) const;

	void drawCorruptionBlock(sf::RenderWindow &window, sf::FloatRect bounds, float lifeFraction, float timer) const;

  private:
	const sf::Texture &idleTexture;
	const sf::Texture &beginRoamingTexture;
	const sf::Texture &roamingTexture;
	const sf::Texture &chargeTexture;
	const sf::Texture &deathTexture;
	sf::Sprite sprite;
};

} // namespace segfault_boss
