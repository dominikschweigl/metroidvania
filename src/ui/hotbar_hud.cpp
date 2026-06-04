#include "hotbar_hud.h"
#include "../core/asset_manager.h"
#include "../core/input_manager.h"
#include "../entities/player/inventory.h"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace {
constexpr sf::Color SLOT_FILL{30, 30, 45, 200};
constexpr sf::Color SLOT_OUTLINE{120, 120, 140};
constexpr sf::Color FLASH_FILL{70, 70, 100, 200};
constexpr sf::Color LABEL_COLOR{180, 180, 200};
constexpr sf::Color EFFECT_BG_COLOR{20, 20, 30, 210};
constexpr sf::Color EFFECT_BG_OUTLINE{90, 90, 110};
constexpr float OUTLINE_THICKNESS = 2.f;
constexpr unsigned LABEL_SIZE = 13;
constexpr float MARGIN_BOTTOM = 16.f;
constexpr float LABEL_PADDING_X = 4.f;
constexpr float LABEL_PADDING_Y = 6.f;
} // namespace

void HotbarHUD::flashSlot(const int slot)
{
	if (slot >= 0 && slot < static_cast<int>(flashTimers_.size()))
		flashTimers_[slot] = FLASH_DURATION;
}

void HotbarHUD::update(const float deltaTime)
{
	for (float &timer : flashTimers_)
		timer = std::max(0.f, timer - deltaTime);
}

void HotbarHUD::draw(sf::RenderWindow &window, const Inventory &inventory, const std::vector<Effect> &effects)
{
	const sf::View previousView = window.getView();
	window.setView(sf::View(sf::FloatRect({0.f, 0.f}, sf::Vector2f(window.getSize()))));

	const sf::Vector2f viewSize = window.getView().getSize();
	const float totalWidth = Inventory::HOTBAR_SIZE * SLOT_SIZE + (Inventory::HOTBAR_SIZE - 1) * SLOT_SPACING;
	const float startX = (viewSize.x - totalWidth) / 2.f;
	const float startY = viewSize.y - SLOT_SIZE - MARGIN_BOTTOM;

	for (int i = 0; i < Inventory::HOTBAR_SIZE; ++i) {
		const sf::Vector2f slotPos{startX + static_cast<float>(i) * (SLOT_SIZE + SLOT_SPACING), startY};
		drawSlotBackground(window, i, slotPos);
		drawItemIcon(window, inventory, i, slotPos);
		drawKeyLabel(window, i, slotPos);
	}

	const float hotbarRightX = startX + totalWidth;
	drawEffectIndicators(window, effects, hotbarRightX, startY);

	window.setView(previousView);
}

void HotbarHUD::drawSlotBackground(sf::RenderWindow &window, const int slotIndex, const sf::Vector2f slotPos) const
{
	sf::RectangleShape slotShape({SLOT_SIZE, SLOT_SIZE});
	slotShape.setOutlineThickness(OUTLINE_THICKNESS);
	slotShape.setOutlineColor(SLOT_OUTLINE);
	slotShape.setFillColor(flashTimers_[slotIndex] > 0.f ? FLASH_FILL : SLOT_FILL);
	slotShape.setPosition(slotPos);
	window.draw(slotShape);
}

void HotbarHUD::drawItemIcon(sf::RenderWindow &window, const Inventory &inventory, const int slotIndex,
                             const sf::Vector2f slotPos) const
{
	const SlotRef hotbarSlot{SlotKind::Hotbar, slotIndex};
	if (!inventory.hasItem(hotbarSlot))
		return;

	const sf::Texture &itemTex = AssetManager::getInstance().getTexture(inventory.itemAt(hotbarSlot).textureAsset());
	sf::Sprite itemSprite(itemTex);
	const sf::Vector2u texSize = itemTex.getSize();
	itemSprite.setScale({ICON_SIZE / static_cast<float>(texSize.x), ICON_SIZE / static_cast<float>(texSize.y)});
	itemSprite.setPosition({slotPos.x + (SLOT_SIZE - ICON_SIZE) / 2, slotPos.y + (SLOT_SIZE - ICON_SIZE) / 2});
	window.draw(itemSprite);
}

void HotbarHUD::drawKeyLabel(sf::RenderWindow &window, const int slotIndex, const sf::Vector2f slotPos) const
{
	const std::string keyName = InputManager::getInstance().inputName(InputManager::hotbarSlotActions()[slotIndex]);
	sf::Text label(AssetManager::getInstance().getFont(UI_FONT), keyName, LABEL_SIZE);
	label.setFillColor(LABEL_COLOR);
	const sf::FloatRect textBounds = label.getLocalBounds();
	label.setPosition({slotPos.x + SLOT_SIZE - textBounds.size.x - LABEL_PADDING_X,
	                   slotPos.y + SLOT_SIZE - textBounds.size.y - LABEL_PADDING_Y});
	window.draw(label);
}

void HotbarHUD::drawEffectIndicators(sf::RenderWindow &window, const std::vector<Effect> &effects,
                                     const float hotbarRightX, const float hotbarTopY) const
{
	constexpr float GAP_ABOVE = 6.f;
	const float y = hotbarTopY - GAP_ABOVE - EFFECT_BG_SIZE;

	for (int i = 0; i < static_cast<int>(effects.size()); ++i) {
		const Effect &effect = effects[i];
		// Stack right-to-left: first effect is rightmost
		const float x =
		    hotbarRightX - static_cast<float>(i + 1) * EFFECT_BG_SIZE - static_cast<float>(i) * EFFECT_SPACING;

		sf::RectangleShape bg({EFFECT_BG_SIZE, EFFECT_BG_SIZE});
		bg.setPosition({x, y});
		bg.setFillColor(EFFECT_BG_COLOR);
		bg.setOutlineColor(EFFECT_BG_OUTLINE);
		bg.setOutlineThickness(1.f);
		window.draw(bg);

		const sf::Texture &tex = AssetManager::getInstance().getTexture(effect.icon());
		sf::Sprite sprite(tex);
		const sf::Vector2u texSize = tex.getSize();
		sprite.setScale(
		    {EFFECT_ICON_SIZE / static_cast<float>(texSize.x), EFFECT_ICON_SIZE / static_cast<float>(texSize.y)});
		sprite.setPosition(
		    {x + (EFFECT_BG_SIZE - EFFECT_ICON_SIZE) / 2.f, y + (EFFECT_BG_SIZE - EFFECT_ICON_SIZE) / 2.f});

		if (effect.remainingDuration < 10.f) {
			const float norm = 0.5f * (1.f + std::sin(effect.remainingDuration * 2.f * std::numbers::pi_v<float>));
			sprite.setColor(sf::Color{255, 255, 255, static_cast<std::uint8_t>(127 + norm * 128.f)});
		}

		window.draw(sprite);
	}
}
