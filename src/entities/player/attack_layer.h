#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

struct AttackDef {
	const sf::Texture *upperTexture;
	int frameCount;
	float frameDuration;
};

class AttackLayer {
  public:
	const sf::Texture swing_texture{"./assets/images/player/attack_swing_upper_body_extended.png"};
	const sf::Texture overhead_texture{"./assets/images/player/attack_overhead_upper_body_extended.png"};

	AttackLayer() : comboChain{{&swing_texture, 8, 0.09f}, {&overhead_texture, 8, 0.09f}} {}

	bool isActive() const noexcept { return comboIndex >= 0; }

	void reset() noexcept
	{
		comboIndex = -1;
		frame = 0;
		frameTimer = 0.f;
		comboQueued = false;
	}

	void trigger()
	{
		if (comboIndex == -1) {
			comboIndex = 0;
			frame = 0;
			frameTimer = 0.f;
			comboQueued = false;
		} else if (comboIndex < static_cast<int>(comboChain.size()) - 1) {
			comboQueued = true;
		}
	}

	void update(float dt)
	{
		if (comboIndex < 0)
			return;
		frameTimer += dt;
		const AttackDef &atk = comboChain[comboIndex];
		if (frameTimer >= atk.frameDuration) {
			frameTimer -= atk.frameDuration;
			++frame;
			if (frame >= atk.frameCount) {
				if (comboQueued && comboIndex < static_cast<int>(comboChain.size()) - 1) {
					comboQueued = false;
					++comboIndex;
					frame = 0;
					frameTimer = 0.f;
				} else {
					comboIndex = -1;
					comboQueued = false;
				}
			}
		}
	}

	void applyAnimation(sf::Sprite &upper, sf::Vector2f scale, sf::Vector2f pos) const
	{
		const AttackDef &atk = comboChain[comboIndex];
		upper.setTexture(*atk.upperTexture);
		upper.setTextureRect(sf::IntRect({frame * 32, 0}, {32, 32}));
		upper.setPosition(pos);
		upper.setScale(scale);
	}

  private:
	std::vector<AttackDef> comboChain;

	int comboIndex = -1;
	int frame = 0;
	float frameTimer = 0.f;
	bool comboQueued = false;
};
