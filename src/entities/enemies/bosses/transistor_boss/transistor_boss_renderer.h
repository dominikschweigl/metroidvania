#pragma once

#include <SFML/Graphics.hpp>

namespace transistor_boss {

// Owns the boss's sprite, its sprite-sheet textures, and all generated visual
// effects (charge aura and capacitor energy beams).
class TransistorBossRenderer {
  public:
	enum class Animation { Roaming, ChargeAttackWindup, ChargeAttack, Recover, Death };

	enum class AuraStyle { Windup, Damage };

	static constexpr int FRAME_SIZE = 128;

	TransistorBossRenderer();
	~TransistorBossRenderer() = default;
	TransistorBossRenderer(const TransistorBossRenderer &) = delete;
	TransistorBossRenderer &operator=(const TransistorBossRenderer &) = delete;
	TransistorBossRenderer(TransistorBossRenderer &&) = delete;
	TransistorBossRenderer &operator=(TransistorBossRenderer &&) = delete;

	void setAnimation(Animation anim, int frame);

	void drawSprite(sf::RenderWindow &window, sf::Vector2f position, float scaleX, sf::Color tint);
	void drawAura(sf::RenderWindow &window, sf::Vector2f center, AuraStyle style, float timer, float radius) const;
	void drawBeam(sf::RenderWindow &window, sf::Vector2f from, sf::Vector2f to, float timer) const;

  private:
	const sf::Texture &roamingTexture;
	const sf::Texture &chargeAttackWindupTexture;
	const sf::Texture &chargeAttackTexture;
	const sf::Texture &recoverTexture;
	const sf::Texture &deathTexture;
	sf::Sprite sprite;
};

} // namespace transistor_boss
