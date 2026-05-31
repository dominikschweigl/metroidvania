#pragma once
#include "../core/scene.h"
#include "../core/scene_stack.h"
#include "../entities/player/inventory.h"
#include "../items/slot_ref.h"
#include <SFML/Graphics.hpp>
#include <optional>

class Player;

class InventoryScene : public Scene {
  public:
	InventoryScene(Player &player, SceneStack &stack, sf::Vector2u windowSize);

	void handleEvent(const sf::Event &event, sf::RenderWindow &window) override;
	void update(float deltaTime) override;
	void draw(sf::RenderWindow &window) override;

	[[nodiscard]] bool isTransparent() const override { return true; }
	[[nodiscard]] bool updateBelow() const override { return false; }

  private:
	struct DragState {
		SlotRef      from{SlotKind::Hat};
		sf::Vector2f mousePos;
	};

	[[nodiscard]] sf::Vector2f              slotScreenPos(SlotRef slot) const noexcept;
	[[nodiscard]] sf::FloatRect             slotBounds(SlotRef slot) const noexcept;
	[[nodiscard]] std::optional<SlotRef>    slotAtPoint(sf::Vector2f point) const noexcept;

	void drawBackground(sf::RenderTarget &target) const;
	void drawPlayerPreview(sf::RenderTarget &target) const;
	void drawSlot(sf::RenderTarget &target, SlotRef slot, bool hovered, bool isDragSource) const;
	void drawAllSlots(sf::RenderTarget &target) const;
	void drawInfoCard(sf::RenderTarget &target, SlotRef slot) const;
	void drawDraggedItem(sf::RenderTarget &target) const;
	void drawText(sf::RenderTarget &target, const std::string &text, sf::Vector2f pos,
	              unsigned int charSize, sf::Color color) const;

	Player              &player_;
	SceneStack          &stack_;
	sf::Vector2u         windowSize_;
	sf::Font             font_;
	bool                 fontLoaded_ = false;
	std::optional<DragState> drag_;
	std::optional<SlotRef>   hovered_;

	// Panel origin (computed in ctor)
	float panelX_ = 0.f;
	float panelY_ = 0.f;

	static constexpr float PANEL_W          = 700.f;
	static constexpr float PANEL_H          = 460.f;
	static constexpr float SLOT_SIZE        = 48.f;
	static constexpr float SLOT_SPACING     =  8.f;
	static constexpr float SECTION_GAP      = 16.f; // gap between preview | perm slots | grid
	static constexpr float PREVIEW_SCALE    =  3.f;
	static constexpr float PREVIEW_BOX_W    = 120.f;
	static constexpr float HAT_SLOT_Y       = 120.f;
	// GUM_SLOT_Y chosen so gum-slot bottom (288+48=336) = grid bottom (GRID_ROWS=4, 120+216=336)
	static constexpr float GUM_SLOT_Y       = 288.f;
	// Preview box spans the same top/bottom as the perm slots
	static constexpr float PREVIEW_BOX_H    = GUM_SLOT_Y + SLOT_SIZE - HAT_SLOT_Y;
	// Character feet centered vertically in the preview box (PLAYER_FRAME_SIZE=32)
	static constexpr float PREVIEW_BOTTOM_Y = HAT_SLOT_Y + (PREVIEW_BOX_H + PREVIEW_SCALE * 32.f) / 2.f;
	static constexpr int   GRID_COLS        = 6;
	static constexpr float MARGIN           =
	    (PANEL_W - PREVIEW_BOX_W - 2.f * SECTION_GAP - SLOT_SIZE
	     - (GRID_COLS * (SLOT_SIZE + SLOT_SPACING) - SLOT_SPACING)) / 2.f;
	static constexpr float PREVIEW_CENTER_X = MARGIN + PREVIEW_BOX_W / 2.f;
	static constexpr float PERM_SLOT_X      = MARGIN + PREVIEW_BOX_W + SECTION_GAP;
	static constexpr float GRID_START_X     = PERM_SLOT_X + SLOT_SIZE + SECTION_GAP;
	static constexpr float GRID_START_Y     = HAT_SLOT_Y;
	static constexpr float HOTBAR_START_X   =
	    (PANEL_W - (Inventory::HOTBAR_SIZE * SLOT_SIZE +
	               (Inventory::HOTBAR_SIZE - 1) * SLOT_SPACING)) / 2.f;
	static constexpr float HOTBAR_Y         = 390.f;
};
