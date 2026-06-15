#pragma once
#include "../effects/effect.h"
#include "health_bar.h"
#include <SFML/Graphics.hpp>
#include <array>
#include <vector>

class Inventory;

class HotbarHUD {
  public:
	HotbarHUD() = default;
	~HotbarHUD() = default;

	HotbarHUD(const HotbarHUD &) = delete;
	HotbarHUD &operator=(const HotbarHUD &) = delete;
	HotbarHUD(HotbarHUD &&) = delete;
	HotbarHUD &operator=(HotbarHUD &&) = delete;

	void update(float deltaTime);
	void draw(sf::RenderWindow &window, const Inventory &inventory, const std::vector<Effect> &effects,
	          const Health &health, bool diskEquipped = false);

	// Call when a hotbar slot is activated to trigger the flash animation.
	void flashSlot(int slot);

  private:
	static constexpr float SLOT_SIZE = 56.f;
	static constexpr float ICON_SIZE = SLOT_SIZE - 10.f;
	static constexpr float SLOT_SPACING = 8.f;
	static constexpr float FLASH_DURATION = 0.15f;
	static constexpr float EFFECT_ICON_SIZE = 24.f;
	static constexpr float EFFECT_BG_SIZE = 30.f;
	static constexpr float EFFECT_SPACING = 6.f;
	static constexpr float INDICATOR_GAP = 6.f;

	HealthBar healthBar_{INDICATOR_GAP, EFFECT_BG_SIZE};
	std::array<float, 5> flashTimers_ = {};

	void drawSlotBackground(sf::RenderWindow &window, int slotIndex, sf::Vector2f slotPos) const;
	void drawItemIcon(sf::RenderWindow &window, const Inventory &inventory, int slotIndex, sf::Vector2f slotPos) const;
	void drawKeyLabel(sf::RenderWindow &window, int slotIndex, sf::Vector2f slotPos) const;
	void drawEffectIndicators(sf::RenderWindow &window, const std::vector<Effect> &effects, float hotbarRightX,
	                          float hotbarTopY) const;
};
