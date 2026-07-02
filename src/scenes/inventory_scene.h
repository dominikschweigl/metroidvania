#pragma once
#include "../core/scene.h"
#include "../core/scene_stack.h"
#include "../effects/effect.h"
#include "../entities/player/inventory.h"
#include "../items/slot_ref.h"
#include <SFML/Graphics.hpp>
#include <optional>
#include <vector>

class InputManager;
class Player;

class InventoryScene : public Scene {
  public:
	InventoryScene(Player &player, SceneStack &stack, const sf::Vector2u windowSize);

	void handleEvent(const sf::Event &event, sf::RenderWindow &window) override;
	void update(const float deltaTime) override;
	void draw(sf::RenderWindow &window) override;

	[[nodiscard]] bool isTransparent() const override { return true; }
	[[nodiscard]] bool updateBelow() const override { return true; }

  private:
	struct DragState {
		SlotRef from{SlotKind::Hat};
		sf::Vector2f mousePos;
	};

	[[nodiscard]] sf::View buildUiView(const sf::RenderWindow &window) const noexcept;
	[[nodiscard]] sf::Vector2f slotScreenPos(const SlotRef slot) const noexcept;
	[[nodiscard]] sf::FloatRect slotBounds(const SlotRef slot) const noexcept;
	[[nodiscard]] std::optional<SlotRef> slotAtPoint(const sf::Vector2f point) const noexcept;

	void updateItemActions(InputManager &input, Inventory &inv);

	void drawBackground(sf::RenderTarget &target) const;
	void drawPlayerPreview(sf::RenderTarget &target) const;
	void drawSlot(sf::RenderTarget &target, const SlotRef slot, const bool hovered, const bool isDragSource) const;
	void drawAllSlots(sf::RenderTarget &target) const;
	void drawInfoCard(sf::RenderTarget &target, const SlotRef slot) const;
	void drawDraggedItem(sf::RenderTarget &target) const;
	void drawActiveEffects(sf::RenderTarget &target, const std::vector<Effect> &effects) const;
	void drawText(sf::RenderTarget &target, const std::string &text, const sf::Vector2f pos,
	              const unsigned int charSize, const sf::Color color) const;
	float drawWrappedText(sf::RenderTarget &target, const std::string &text, const sf::Vector2f pos,
	                      const unsigned int charSize, const sf::Color color, const float maxWidth) const;

	Player &player_;
	SceneStack &stack_;
	sf::Vector2u windowSize_;
	std::optional<DragState> drag_;
	std::optional<SlotRef> hovered_;

	// Panel origin (computed in ctor)
	float panelX_ = 0.f;
	float panelY_ = 0.f;

	static constexpr float PANEL_W = 700.f;
	static constexpr float PANEL_H = 460.f;
	static constexpr float SLOT_SIZE = 48.f;
	static constexpr float SLOT_SPACING = 8.f;
	static constexpr float SECTION_GAP = 16.f; // gap between preview | perm slots | grid
	static constexpr float PREVIEW_SCALE = 3.f;
	static constexpr float PREVIEW_BOX_W = 120.f;
	static constexpr float HAT_SLOT_Y = 120.f;
	// GUM_SLOT_Y chosen so gum-slot bottom (288+48=336) = grid bottom (GRID_ROWS=4, 120+216=336)
	static constexpr float GUM_SLOT_Y = 288.f;
	static constexpr float BACKUP_SLOT_Y = (HAT_SLOT_Y + GUM_SLOT_Y) / 2.f;

	// Active effects side panel (to the right of the main panel)
	static constexpr float EFFECTS_PANEL_GAP = 12.f;
	static constexpr float EFFECTS_PANEL_W = 220.f;
	static constexpr float MIN_CANVAS_W = PANEL_W + 2.f * (EFFECTS_PANEL_GAP + EFFECTS_PANEL_W) + 40.f;
	static constexpr float EFFECTS_CARD_H = 48.f;
	static constexpr float EFFECTS_CARD_ICON_SIZE = 32.f;
	// Preview box spans the same top/bottom as the perm slots
	static constexpr float PREVIEW_BOX_H = GUM_SLOT_Y + SLOT_SIZE - HAT_SLOT_Y;
	// Character feet centered vertically in the preview box (PLAYER_FRAME_SIZE=32)
	static constexpr float PREVIEW_BOTTOM_Y = HAT_SLOT_Y + (PREVIEW_BOX_H + PREVIEW_SCALE * 32.f) / 2.f;
	static constexpr float PREVIEW_CENTER_X = 146.f; // = 86 (margin) + PREVIEW_BOX_W/2
	static constexpr float PERM_SLOT_X = 222.f;      // = 86 + PREVIEW_BOX_W + SECTION_GAP
	static constexpr int GRID_COLS = 6;
	static constexpr float GRID_START_X = 286.f; // = PERM_SLOT_X + SLOT_SIZE + SECTION_GAP
	static constexpr float GRID_START_Y = HAT_SLOT_Y;
	static constexpr float HOTBAR_START_X =
	    (PANEL_W - (Inventory::HOTBAR_SIZE * SLOT_SIZE + (Inventory::HOTBAR_SIZE - 1) * SLOT_SPACING)) / 2.f;
	static constexpr float HOTBAR_Y = 390.f;
};
