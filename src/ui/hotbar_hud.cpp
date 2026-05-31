#include "hotbar_hud.h"
#include "../core/asset_manager.h"
#include "../core/input_manager.h"
#include "../entities/player/inventory.h"
#include <algorithm>

namespace {
constexpr sf::Color SLOT_FILL{30, 30, 45, 200};
constexpr sf::Color SLOT_OUTLINE{120, 120, 140};
constexpr sf::Color FLASH_FILL{70, 70, 100, 200};
constexpr sf::Color LABEL_COLOR{180, 180, 200};
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
	for (auto &timer : flashTimers_)
		timer = std::max(0.f, timer - deltaTime);
}

void HotbarHUD::draw(sf::RenderWindow &window, const Inventory &inventory)
{
	const sf::View previousView = window.getView();
	window.setView(window.getDefaultView());

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
