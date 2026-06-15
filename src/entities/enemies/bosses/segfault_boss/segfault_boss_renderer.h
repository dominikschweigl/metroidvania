#pragma once

#include <SFML/Graphics.hpp>

namespace segfault_boss {

// Owns the boss's sprite, its sprite-sheet textures, and all generated visuals
class SegfaultBossRenderer {
  public:
	enum class Animation { Idle, Roaming, Death };

	// The currently used frames are 32x32. Upscaled by 2 for bigger size.
	static constexpr int FRAME_SIZE = 32;
	static constexpr float SPRITE_SCALE = 2.f;

	SegfaultBossRenderer();
	~SegfaultBossRenderer() = default;
	SegfaultBossRenderer(const SegfaultBossRenderer &) = delete;
	SegfaultBossRenderer &operator=(const SegfaultBossRenderer &) = delete;
	SegfaultBossRenderer(SegfaultBossRenderer &&) = delete;
	SegfaultBossRenderer &operator=(SegfaultBossRenderer &&) = delete;

	void setAnimation(Animation anim, int frame);

	void drawSprite(sf::RenderWindow &window, sf::Vector2f position, float scaleX, sf::Color tint);

  private:
	const sf::Texture &idleTexture;
	const sf::Texture &walkTexture;
	const sf::Texture &runTexture;
	sf::Sprite sprite;
};

} // namespace segfault_boss
