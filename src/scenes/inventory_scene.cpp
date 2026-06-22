#include "inventory_scene.h"
#include "../core/asset_manager.h"
#include "../core/input_manager.h"
#include "../entities/player/inventory.h"
#include "../entities/player/player.h"
#include <cmath>

namespace {
constexpr sf::Color PANEL_BG{15, 15, 25, 220};
constexpr sf::Color PANEL_BORDER{180, 180, 200};
constexpr sf::Color SLOT_FILL{30, 30, 45, 255};
constexpr sf::Color SLOT_PERM_FILL{25, 25, 55, 255};
constexpr sf::Color SLOT_OUTLINE{90, 90, 110};
constexpr sf::Color SLOT_HOVERED_OUTLINE{200, 200, 220};
constexpr sf::Color SLOT_DRAG_SOURCE_OUTLINE{255, 255, 255};
constexpr sf::Color SLOT_SELECTED_FILL{70, 70, 100, 255};
constexpr sf::Color CARD_BG{20, 20, 35, 240};
constexpr sf::Color CARD_BORDER{140, 140, 170};
constexpr sf::Color TEXT_TITLE{240, 240, 240};
constexpr sf::Color TEXT_BODY{180, 180, 200};
constexpr sf::Color TEXT_EFFECT{120, 220, 120};
constexpr float OUTLINE_THICK = 2.f;
constexpr sf::Color HOTBAR_LABEL_COLOR{180, 180, 200};
constexpr unsigned int HOTBAR_LABEL_SIZE = 13;
constexpr float HOTBAR_LABEL_PADDING_X = 4.f;
constexpr float HOTBAR_LABEL_PADDING_Y = 6.f;

constexpr int PLAYER_FRAME_SIZE = 32;
} // namespace

InventoryScene::InventoryScene(Player &player, SceneStack &stack, const sf::Vector2u windowSize)
    : player_(player), stack_(stack), windowSize_(windowSize)
{
	panelX_ = (static_cast<float>(windowSize.x) - PANEL_W) / 2.f;
	panelY_ = (static_cast<float>(windowSize.y) - PANEL_H) / 2.f;
}

// --- View ---

sf::View InventoryScene::buildUiView(const sf::RenderWindow &window) const noexcept
{
	const float actualW = static_cast<float>(window.getSize().x);
	const float actualH = static_cast<float>(window.getSize().y);
	const float scale = std::min(1.f, actualW / MIN_CANVAS_W);
	return sf::View(sf::FloatRect({0.f, 0.f}, {actualW / scale, actualH / scale}));
}

// --- Layout ---

sf::Vector2f InventoryScene::slotScreenPos(const SlotRef slot) const noexcept
{
	switch (slot.kind) {
	case SlotKind::Hat:
		return {panelX_ + PERM_SLOT_X, panelY_ + HAT_SLOT_Y};
	case SlotKind::Gum:
		return {panelX_ + PERM_SLOT_X, panelY_ + GUM_SLOT_Y};
	case SlotKind::Backup:
		return {panelX_ + PERM_SLOT_X, panelY_ + BACKUP_SLOT_Y};
	case SlotKind::Grid: {
		const int col = slot.index % GRID_COLS;
		const int row = slot.index / GRID_COLS;
		return {panelX_ + GRID_START_X + static_cast<float>(col) * (SLOT_SIZE + SLOT_SPACING),
		        panelY_ + GRID_START_Y + static_cast<float>(row) * (SLOT_SIZE + SLOT_SPACING)};
	}
	case SlotKind::Hotbar:
		return {panelX_ + HOTBAR_START_X + static_cast<float>(slot.index) * (SLOT_SIZE + SLOT_SPACING),
		        panelY_ + HOTBAR_Y};
	}
	return {};
}

sf::FloatRect InventoryScene::slotBounds(const SlotRef slot) const noexcept
{
	return {slotScreenPos(slot), {SLOT_SIZE, SLOT_SIZE}};
}

std::optional<SlotRef> InventoryScene::slotAtPoint(const sf::Vector2f point) const noexcept
{
	for (const SlotRef &slot : Inventory::slots()) {
		if (slotBounds(slot).contains(point))
			return slot;
	}
	return std::nullopt;
}

// --- Event handling ---

void InventoryScene::handleEvent(const sf::Event &event, sf::RenderWindow &window)
{
	Inventory &inv = player_.inventory();
	const sf::View uiView = buildUiView(window);

	auto toViewCoords = [&](sf::Vector2i pixelPos) { return window.mapPixelToCoords(pixelPos, uiView); };

	if (const auto *moved = event.getIf<sf::Event::MouseMoved>()) {
		const sf::Vector2f mousePos = toViewCoords(moved->position);
		hovered_ = slotAtPoint(mousePos);
		if (drag_)
			drag_->mousePos = mousePos;
	}

	if (const auto *pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
		if (pressed->button == sf::Mouse::Button::Left) {
			const sf::Vector2f mousePos = toViewCoords(pressed->position);
			if (const auto slot = slotAtPoint(mousePos)) {
				if (inv.hasItem(*slot)) {
					const bool shiftHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::LShift)
					                       || sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::RShift);
					if (shiftHeld) {
						const std::span<const SlotRef> targets =
						    (slot->kind == SlotKind::Hotbar) ? Inventory::gridSlots() : Inventory::hotbarSlots();
						for (const SlotRef &target : targets) {
							if (!inv.hasItem(target)) {
								inv.moveItem(*slot, target);
								break;
							}
						}
					} else {
						drag_ = DragState{*slot, mousePos};
					}
				}
			}
		}
	}

	if (const auto *released = event.getIf<sf::Event::MouseButtonReleased>()) {
		if (released->button == sf::Mouse::Button::Left && drag_) {
			const sf::Vector2f mousePos = toViewCoords(released->position);
			if (const auto target = slotAtPoint(mousePos)) {
				inv.moveItem(drag_->from, *target);
			}
			drag_.reset();
		}
	}
}

void InventoryScene::update(const float /*deltaTime*/)
{
	InputManager &input = InputManager::getInstance();
	Inventory &inv = player_.inventory();

	updateItemActions(input, inv);

	input.suppressPlayerActions();

	// Consume both close keys so GameScene::update() never sees them this frame.
	const bool wantsClose = input.consume(MenuAction::Back) || input.consume(GameAction::OpenInventory);
	if (wantsClose) {
		if (drag_)
			drag_.reset(); // cancel drag first; user presses again to actually close
		else
			stack_.pop();
		return;
	}
}

void InventoryScene::updateItemActions(InputManager &input, Inventory &inv)
{
	// Hotbar slot keys: move the hovered item into the pressed slot, then block the key from GameScene.
	const std::span<const GameAction> slotActions = InputManager::hotbarSlotActions();
	for (int i = 0; i < static_cast<int>(slotActions.size()); ++i) {
		const bool pressed = input.consume(slotActions[i]);

		if (pressed && hovered_ && !drag_) {
			if (inv.hasItem(*hovered_)) {
				inv.moveItem(*hovered_, SlotRef{SlotKind::Hotbar, i});
			} else {
				inv.moveItem(SlotRef{SlotKind::Hotbar, i}, *hovered_);
			}
		}
	}

	if (input.wasPressed(GameAction::UseItem) && hovered_ && !drag_)
		inv.interact(*hovered_, player_);
}

// --- Drawing ---

void InventoryScene::drawText(sf::RenderTarget &target, const std::string &text, const sf::Vector2f pos,
                              const unsigned int charSize, const sf::Color color) const
{
	sf::Text label(AssetManager::getInstance().getFont(UI_FONT), text, charSize);
	label.setFillColor(color);
	const sf::FloatRect bounds = label.getLocalBounds();
	label.setOrigin({bounds.position.x, bounds.position.y});
	label.setPosition(pos);
	target.draw(label);
}

float InventoryScene::drawWrappedText(sf::RenderTarget &target, const std::string &text, const sf::Vector2f pos,
                                      const unsigned int charSize, const sf::Color color, const float maxWidth) const
{
	const sf::Font &font = AssetManager::getInstance().getFont(UI_FONT);
	const float lineHeight = static_cast<float>(charSize) * 1.4f;

	std::string currentLine;
	std::string currentWord;
	float currentY = pos.y;

	auto flushWord = [&]() {
		if (currentWord.empty())
			return;
		std::string testLine = currentLine.empty() ? currentWord : currentLine + " " + currentWord;
		sf::Text test(font, testLine, charSize);
		if (!currentLine.empty() && test.getLocalBounds().size.x > maxWidth) {
			drawText(target, currentLine, {pos.x, currentY}, charSize, color);
			currentLine = currentWord;
			currentY += lineHeight;
		} else {
			currentLine = testLine;
		}
		currentWord.clear();
	};

	for (const char ch : text) {
		if (ch == ' ' || ch == '\n')
			flushWord();
		else
			currentWord += ch;
	}
	flushWord();

	if (!currentLine.empty()) {
		drawText(target, currentLine, {pos.x, currentY}, charSize, color);
		currentY += lineHeight;
	}

	return currentY;
}

void InventoryScene::drawBackground(sf::RenderTarget &target) const
{
	sf::RectangleShape panel({PANEL_W, PANEL_H});
	panel.setPosition({panelX_, panelY_});
	panel.setFillColor(PANEL_BG);
	panel.setOutlineColor(PANEL_BORDER);
	panel.setOutlineThickness(OUTLINE_THICK);
	target.draw(panel);

	drawText(target, "INVENTORY", {panelX_ + 16.f, panelY_ + 12.f}, 24, TEXT_TITLE);
	drawText(target, "Hat", {panelX_ + PERM_SLOT_X, panelY_ + HAT_SLOT_Y - 20.f}, 13, TEXT_BODY);
	drawText(target, "Backup", {panelX_ + PERM_SLOT_X, panelY_ + BACKUP_SLOT_Y - 20.f}, 13, TEXT_BODY);
	drawText(target, "Gum", {panelX_ + PERM_SLOT_X, panelY_ + GUM_SLOT_Y - 20.f}, 13, TEXT_BODY);
	drawText(target, "Items", {panelX_ + GRID_START_X, panelY_ + GRID_START_Y - 20.f}, 13, TEXT_BODY);
	drawText(target, "Hotbar", {panelX_ + HOTBAR_START_X, panelY_ + HOTBAR_Y - 20.f}, 13, TEXT_BODY);
}

void InventoryScene::drawPlayerPreview(sf::RenderTarget &target) const
{
	const float centerX = panelX_ + PREVIEW_CENTER_X;
	const float bottomY = panelY_ + PREVIEW_BOTTOM_Y;
	const float scale = PREVIEW_SCALE;

	const float charH = static_cast<float>(PLAYER_FRAME_SIZE) * scale;
	const float boxLeft = centerX - PREVIEW_BOX_W / 2.f;
	const float boxTop = bottomY - charH - (PREVIEW_BOX_H - charH) / 2.f;

	sf::RectangleShape previewBox({PREVIEW_BOX_W, PREVIEW_BOX_H});
	previewBox.setPosition({boxLeft, boxTop});
	previewBox.setFillColor(SLOT_PERM_FILL);
	previewBox.setOutlineColor(SLOT_OUTLINE);
	previewBox.setOutlineThickness(OUTLINE_THICK);
	target.draw(previewBox);

	const sf::IntRect frame0({0, 0}, {PLAYER_FRAME_SIZE, PLAYER_FRAME_SIZE});
	const sf::Vector2f origin{PLAYER_FRAME_SIZE / 2.f, static_cast<float>(PLAYER_FRAME_SIZE)};

	auto drawLayer = [&](TextureAsset asset) {
		sf::Sprite sprite(AssetManager::getInstance().getTexture(asset));
		sprite.setTextureRect(frame0);
		sprite.setOrigin(origin);
		sprite.setScale({scale, scale});
		sprite.setPosition({centerX, bottomY});
		target.draw(sprite);
	};

	drawLayer(PLAYER_IDLE_LOWER_BODY);
	drawLayer(PLAYER_IDLE_UPPER_BODY);

	const TextureAsset headAsset = player_.inventory().hasHat() ? PLAYER_HEAD_HAT : PLAYER_HEAD;
	sf::Sprite headSprite(AssetManager::getInstance().getTexture(headAsset));
	headSprite.setTextureRect(frame0);
	headSprite.setOrigin(origin);
	headSprite.setScale({scale, scale});
	headSprite.setPosition({centerX, bottomY});
	target.draw(headSprite);
}

void InventoryScene::drawSlot(sf::RenderTarget &target, const SlotRef slot, const bool hovered,
                              const bool isDragSource) const
{
	const sf::Vector2f pos = slotScreenPos(slot);
	bool isPerm = false;
	for (const SlotRef &equipSlot : Inventory::equipmentSlots()) {
		if (equipSlot.kind == slot.kind) {
			isPerm = true;
			break;
		}
	}

	sf::RectangleShape shape({SLOT_SIZE, SLOT_SIZE});
	shape.setPosition(pos);
	shape.setFillColor(hovered ? SLOT_SELECTED_FILL : (isPerm ? SLOT_PERM_FILL : SLOT_FILL));
	if (isDragSource)
		shape.setOutlineColor(SLOT_DRAG_SOURCE_OUTLINE);
	else if (hovered)
		shape.setOutlineColor(SLOT_HOVERED_OUTLINE);
	else
		shape.setOutlineColor(SLOT_OUTLINE);
	shape.setOutlineThickness(OUTLINE_THICK);
	target.draw(shape);

	const Inventory &inv = player_.inventory();
	const bool showItem = inv.hasItem(slot) && !isDragSource;
	if (showItem) {
		const sf::Texture &tex = AssetManager::getInstance().getTexture(inv.itemAt(slot).textureAsset());
		sf::Sprite sprite(tex);
		const sf::Vector2u texSize = tex.getSize();
		const float iconSize = SLOT_SIZE - 10.f;
		sprite.setScale({iconSize / static_cast<float>(texSize.x), iconSize / static_cast<float>(texSize.y)});
		sprite.setPosition({pos.x + 5.f, pos.y + 5.f});
		target.draw(sprite);
	}

	if (slot.kind == SlotKind::Hotbar) {
		const std::string keyName =
		    InputManager::getInstance().inputName(InputManager::hotbarSlotActions()[slot.index]);
		sf::Text label(AssetManager::getInstance().getFont(UI_FONT), keyName, HOTBAR_LABEL_SIZE);
		label.setFillColor(HOTBAR_LABEL_COLOR);
		const sf::FloatRect textBounds = label.getLocalBounds();
		label.setPosition({pos.x + SLOT_SIZE - textBounds.size.x - HOTBAR_LABEL_PADDING_X,
		                   pos.y + SLOT_SIZE - textBounds.size.y - HOTBAR_LABEL_PADDING_Y});
		target.draw(label);
	}
}

void InventoryScene::drawAllSlots(sf::RenderTarget &target) const
{
	for (const SlotRef &slot : Inventory::slots()) {
		const bool isHovered = hovered_ && hovered_->kind == slot.kind && hovered_->index == slot.index;
		const bool isDragSource = drag_ && drag_->from.kind == slot.kind && drag_->from.index == slot.index;
		drawSlot(target, slot, isHovered, isDragSource);
	}
}

void InventoryScene::drawInfoCard(sf::RenderTarget &target, const SlotRef slot) const
{
	const Inventory &inv = player_.inventory();
	if (!inv.hasItem(slot))
		return;

	const ItemInfo info = inv.itemAt(slot).info();
	const sf::Vector2f slotPos = slotScreenPos(slot);

	constexpr float CARD_W = 210.f;
	constexpr float CARD_H = 110.f;
	constexpr float CARD_PAD = 8.f;
	constexpr float LINE_H_SM = 18.f;
	constexpr float LINE_H_LG = 26.f;

	// Place card to the right of the slot, or left if near window edge
	float cardX = slotPos.x + SLOT_SIZE + 6.f;
	if (cardX + CARD_W > panelX_ + PANEL_W)
		cardX = slotPos.x - CARD_W - 6.f;
	const float cardY = slotPos.y;

	sf::RectangleShape card({CARD_W, CARD_H});
	card.setPosition({cardX, cardY});
	card.setFillColor(CARD_BG);
	card.setOutlineColor(CARD_BORDER);
	card.setOutlineThickness(OUTLINE_THICK);
	target.draw(card);

	drawText(target, std::string(info.name), {cardX + CARD_PAD, cardY + CARD_PAD}, 14, TEXT_TITLE);
	drawWrappedText(target, std::string(info.description), {cardX + CARD_PAD, cardY + CARD_PAD + LINE_H_LG}, 12,
	                TEXT_BODY, CARD_W - 2.f * CARD_PAD);

	// Divider
	sf::RectangleShape divider({CARD_W - 2.f * CARD_PAD, 1.f});
	divider.setPosition({cardX + CARD_PAD, cardY + CARD_H - LINE_H_SM - CARD_PAD - 4.f});
	divider.setFillColor(CARD_BORDER);
	target.draw(divider);

	drawText(target, std::string(info.effect), {cardX + CARD_PAD, cardY + CARD_H - LINE_H_SM - CARD_PAD + 2.f}, 12,
	         TEXT_EFFECT);
}

void InventoryScene::drawDraggedItem(sf::RenderTarget &target) const
{
	if (!drag_)
		return;
	const Inventory &inv = player_.inventory();
	if (!inv.hasItem(drag_->from))
		return;

	const sf::Texture &tex = AssetManager::getInstance().getTexture(inv.itemAt(drag_->from).textureAsset());
	sf::Sprite sprite(tex);
	const sf::Vector2u texSize = tex.getSize();
	const float iconSize = SLOT_SIZE - 6.f;
	sprite.setScale({iconSize / static_cast<float>(texSize.x), iconSize / static_cast<float>(texSize.y)});
	sprite.setPosition({drag_->mousePos.x - iconSize / 2.f, drag_->mousePos.y - iconSize / 2.f});
	target.draw(sprite);
}

void InventoryScene::drawActiveEffects(sf::RenderTarget &target, const std::vector<Effect> &effects) const
{
	const float sideX = panelX_ + PANEL_W + EFFECTS_PANEL_GAP;
	const float sideY = panelY_;

	sf::RectangleShape sidePanel({EFFECTS_PANEL_W, PANEL_H});
	sidePanel.setPosition({sideX, sideY});
	sidePanel.setFillColor(PANEL_BG);
	sidePanel.setOutlineColor(PANEL_BORDER);
	sidePanel.setOutlineThickness(OUTLINE_THICK);
	target.draw(sidePanel);

	constexpr float TITLE_PAD = 12.f;
	constexpr float TITLE_H = 22.f;
	drawText(target, "Active Effects", {sideX + TITLE_PAD, sideY + TITLE_PAD}, 14, TEXT_TITLE);

	constexpr float CARDS_START_Y = TITLE_PAD + TITLE_H + 8.f;
	constexpr float CARD_PAD = 8.f;
	constexpr float CARD_W = EFFECTS_PANEL_W - 2.f * CARD_PAD;

	if (effects.empty()) {
		drawText(target, "None", {sideX + CARD_PAD, sideY + CARDS_START_Y}, 12, sf::Color{100, 100, 120, 200});
		return;
	}

	for (int i = 0; i < static_cast<int>(effects.size()); ++i) {
		const Effect &effect = effects[i];
		const float cardY = sideY + CARDS_START_Y + static_cast<float>(i) * (EFFECTS_CARD_H + CARD_PAD);

		sf::RectangleShape card({CARD_W, EFFECTS_CARD_H});
		card.setPosition({sideX + CARD_PAD, cardY});
		card.setFillColor(CARD_BG);
		card.setOutlineColor(CARD_BORDER);
		card.setOutlineThickness(OUTLINE_THICK);
		target.draw(card);

		// Icon (16x16, left side, vertically centred)
		const sf::Texture &tex = AssetManager::getInstance().getTexture(effect.icon());
		sf::Sprite sprite(tex);
		const sf::Vector2u texSize = tex.getSize();
		sprite.setScale({EFFECTS_CARD_ICON_SIZE / static_cast<float>(texSize.x),
		                 EFFECTS_CARD_ICON_SIZE / static_cast<float>(texSize.y)});
		sprite.setPosition({sideX + CARD_PAD + (EFFECTS_CARD_H - EFFECTS_CARD_ICON_SIZE) / 2.f,
		                    cardY + (EFFECTS_CARD_H - EFFECTS_CARD_ICON_SIZE) / 2.f});
		target.draw(sprite);

		// Text to the right of the icon
		const float textX = sideX + CARD_PAD + EFFECTS_CARD_H + 4.f;
		drawText(target, std::string(effect.name()), {textX, cardY + 6.f}, 13, TEXT_TITLE);

		const int remainingSeconds = static_cast<int>(std::ceil(effect.remainingDuration));
		drawText(target, std::to_string(remainingSeconds) + " s remaining", {textX, cardY + 26.f}, 11, TEXT_BODY);
	}
}

void InventoryScene::draw(sf::RenderWindow &window)
{
	const sf::View previousView = window.getView();
	window.setView(buildUiView(window));

	// Recompute panel origin from the view that's actually used for rendering
	// so that drawing positions and mouse hit-testing always share the same space.
	const sf::Vector2f viewSize = window.getView().getSize();
	panelX_ = (viewSize.x - PANEL_W) / 2.f;
	panelY_ = (viewSize.y - PANEL_H) / 2.f;

	drawBackground(window);
	drawPlayerPreview(window);
	drawAllSlots(window);

	if (hovered_ && !drag_)
		drawInfoCard(window, *hovered_);

	drawDraggedItem(window);
	if (!player_.activeEffects().empty())
		drawActiveEffects(window, player_.activeEffects());

	window.setView(previousView);
}
