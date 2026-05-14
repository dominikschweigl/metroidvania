#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <vector>

struct AttackDef {
	std::reference_wrapper<const sf::Texture> upperTexture;
	int frameCount;
	float frameDuration;
};

class MeleeAttack {
  public:
	const sf::Texture &swing_texture;
	const sf::Texture &overhead_texture;

	MeleeAttack();

	[[nodiscard]] bool isMeleeActive() const noexcept { return comboIndex >= 0; }

	void reset() noexcept;
	void trigger();
	void update(float dt);
	void applyAnimation(sf::Sprite &upper, sf::Vector2f scale, sf::Vector2f pos) const;

  private:
	std::vector<AttackDef> comboChain;

	int comboIndex = -1;
	int frame = 0;
	float frameTimer = 0.f;
	bool comboQueued = false;
};
