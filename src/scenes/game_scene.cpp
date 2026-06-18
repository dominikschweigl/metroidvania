#include "game_scene.h"
#include "../core/input_manager.h"
#include "../entities/enemies/bosses/segfault_boss/segfault_boss.h"
#include "../entities/enemies/bosses/transistor_boss/transistor_boss.h"
#include "../items/backup_disk_item.h"
#include "../items/chewing_gum_item.h"
#include "../items/damage_potion_item.h"
#include "../items/hat_item.h"
#include "../items/healing_potion_item.h"
#include "../items/jump_potion_item.h"
#include "../items/resistance_potion_item.h"
#include "../items/speed_potion_item.h"
#include "../items/usb_key_item.h"
#include "inventory_scene.h"
#include "menus/bluescreen_menu.h"
#include "menus/game_over_menu.h"
#include "menus/pause_menu.h"
#include <algorithm>
#include <cstdint>
#include <vector>

GameScene::GameScene(SceneStack &sceneStack, sf::RenderWindow &window, std::string gameName, bool makeNewGame)
    : sceneStack_(sceneStack), window_(window), world_(gameName)
{
	AudioManager::getInstance().playMusic(MusicTrack::GAME_THEME);

	const sf::Vector2u windowSize = window.getSize();
	view_.setSize({static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)});
	view_.setCenter(view_.getSize() / 2.f);

	world_.loadRoom("start_room", "data/maps/start_room.tmj");
	world_.loadRoom("1", "data/maps/1.tmj");
	world_.loadRoom("2", "data/maps/2.tmj");
	world_.loadRoom("3", "data/maps/3.tmj");
	world_.loadRoom("4", "data/maps/4.tmj");
	world_.loadRoom("5", "data/maps/5.tmj");
	world_.loadRoom("6", "data/maps/6.tmj");
	world_.loadRoom("7", "data/maps/7.tmj");
	world_.loadRoom("8", "data/maps/8.tmj");

	if (makeNewGame)
		this->newGame(window);
	else
		this->loadGame(window);
}

void GameScene::newGame(sf::RenderWindow &window)
{
	player_.setPosition(world_.getCurrentRoom()->playerSpawns[0]);

	view_.setCenter(player_.getPosition());
}

void GameScene::loadGame(sf::RenderWindow &window)
{
	world_.loadWorldData(player_);

	view_.setCenter(player_.getPosition());
}

void GameScene::handleEvent(const sf::Event &event, sf::RenderWindow &window)
{
	if (const auto *resized = event.getIf<sf::Event::Resized>()) {
		view_.setSize({static_cast<float>(resized->size.x), static_cast<float>(resized->size.y)});
		view_.setCenter(player_.getPosition());
		window.setView(view_);
	}
}

void GameScene::update(float deltaTime)
{
	const InputManager &input = InputManager::getInstance();

	if (input.wasPressed(GameAction::ZoomIn))
		zoomFactor_ *= 0.9f;
	if (input.wasPressed(GameAction::ZoomOut))
		zoomFactor_ *= 1.1f;
	if (input.wasPressed(MenuAction::Back))
		sceneStack_.push([&stack = sceneStack_, &window = window_]() { return makePauseMenu(stack, window); });
	if (input.wasPressed(GameAction::ToggleDebugHitboxes))
		showDebugHitboxes_ = !showDebugHitboxes_;

	if (input.wasPressed(GameAction::OpenInventory))
		sceneStack_.push([&player = player_, &stack = sceneStack_, windowSize = window_.getSize()]() {
			return std::make_unique<InventoryScene>(player, stack, windowSize);
		});

	for (int i = 0; i < Inventory::HOTBAR_SIZE; ++i) {
		if (input.wasPressed(InputManager::hotbarSlotActions()[i])) {
			hotbarHud_.flashSlot(i);
			player_.useHotbarSlot(i);
		}
	}

	const bool attackTriggered = input.wasPressed(GameAction::AttackMelee);
	const bool hatThrowTriggered = input.wasPressed(GameAction::ThrowHat);

	player_.update(deltaTime, world_, attackTriggered, hatThrowTriggered);

	// Update all alive enemies; collect drops before removing dead ones.
	for (auto &enemy : world_.getCurrentRoom()->enemies_)
		enemy->update(deltaTime, world_, player_.getPosition(), player_.getBounds());
	for (auto &enemy : world_.getCurrentRoom()->enemies_) {
		if (!enemy->isAlive()) {
			for (std::unique_ptr<Item> &drop : enemy->rollDrops()) {
				auto droppedItem = std::make_unique<WorldItem>(enemy->getPosition(), std::move(drop));
				world_.getCurrentRoom()->appendItem(droppedItem);
			}
			combat_.clearVictim(&enemy->health);
		}
	}

	// The segfault boss interrupts the fight with a "bluescreen" between stages.
	for (auto &enemy : world_.getCurrentRoom()->enemies_) {
		auto *segfaultBoss = dynamic_cast<SegfaultBoss *>(enemy.get());
		if (segfaultBoss != nullptr && segfaultBoss->consumeBluescreenRequest())
			sceneStack_.push([&stack = sceneStack_, &window = window_]() { return makeBluescreenMenu(stack, window); });
	}

	// Update world items and check for player pickup.
	for (auto &item : world_.getCurrentRoom()->items_)
		item->update(deltaTime, world_);
	for (const std::unique_ptr<WorldItem> &worldItem : world_.getCurrentRoom()->items_) {
		std::unique_ptr<Item> collected = worldItem->tryCollect(player_.getBounds());
		if (collected)
			player_.inventory().addItem(std::move(collected));
	}

	world_.update(deltaTime, player_.getBounds());

	hitboxes_.clear();
	hurtboxes_.clear();
	player_.collectHitboxes(hitboxes_);
	player_.collectHurtboxes(hurtboxes_);
	for (auto &enemy : world_.getCurrentRoom()->enemies_) {
		enemy->collectHitboxes(hitboxes_);
		enemy->collectHurtboxes(hurtboxes_);
	}

	combat_.resolve(hitboxes_, hurtboxes_);

	std::vector<std::uint32_t> endedSourceIds;
	player_.drainEndedSourceIds(endedSourceIds);
	for (auto &enemy : world_.getCurrentRoom()->enemies_)
		enemy->drainEndedSourceIds(endedSourceIds);
	for (const std::uint32_t id : endedSourceIds)
		combat_.clearSource(id);

	hotbarHud_.update(deltaTime);
	resetPlayerIfOutOfBounds();

	// Backup Disk revive: intercept death before the game-over check
	if (!player_.isAlive() && player_.inventory().hasBackup()) {
		player_.heal(1);
		player_.inventory().clearSlot({SlotKind::Backup, 0});
		player_.triggerIframes();
	}

	// Check for player death
	if (!player_.isAlive()) {
		sceneStack_.push([&stack = sceneStack_, &window = window_]() { return makeGameOverMenu(stack, window); });
	}

	if (input.wasPressed(GameAction::Interact) && world_.getCurrentRoom()->isAllowedLeaving()) {
		std::optional<std::pair<std::string, int>> touchingDoorTargetRoom =
		    world_.getTouchingDoorTargetRoom(player_.getBounds());
		if (touchingDoorTargetRoom && world_.getCurrentRoom()->isAllowedLeaving()) {
			world_.setCurrentRoom(touchingDoorTargetRoom.value().first);
			player_.setPosition(world_.getCurrentRoom()->playerSpawns[touchingDoorTargetRoom.value().second]);
			if (world_.getCurrentRoomId() == "boss_room") {
				AudioManager::getInstance().playMusic(MusicTrack::AREA_1_BOSS_THEME);
			} else {
				AudioManager::getInstance().playMusic(MusicTrack::GAME_THEME);
			}
		} else if (world_.isTouchingSavepoint(player_.getBounds())) {
			world_.saveWorldData(player_);
		}
	}

	view_.setCenter(player_.getPosition());
}

void GameScene::draw(sf::RenderWindow &window)
{
	sf::Vector2u windowSize = window.getSize();
	view_.setSize({windowSize.x * zoomFactor_, windowSize.y * zoomFactor_});
	window.setView(view_);
	window.clear({0, 0, 0});
	world_.draw(window, view_, player_.getBounds());
	for (auto &item : world_.getCurrentRoom()->items_)
		item->draw(window);
	player_.draw(window);
	for (auto &enemy : world_.getCurrentRoom()->enemies_)
		enemy->draw(window);

	if (showDebugHitboxes_)
		drawDebugHitboxes(window);

	hotbarHud_.draw(window, player_.inventory(), player_.activeEffects(), player_.health,
	                player_.inventory().hasBackup());
}

void GameScene::drawDebugHitboxes(sf::RenderWindow &window)
{
	sf::RectangleShape outline;
	outline.setFillColor(sf::Color::Transparent);
	outline.setOutlineThickness(1.f);

	outline.setOutlineColor(sf::Color::Green);
	for (const Hurtbox &hurt : hurtboxes_) {
		outline.setPosition(hurt.bounds.position);
		outline.setSize(hurt.bounds.size);
		window.draw(outline);
	}

	outline.setOutlineColor(sf::Color::Red);
	for (const Hitbox &hit : hitboxes_) {
		outline.setPosition(hit.bounds.position);
		outline.setSize(hit.bounds.size);
		window.draw(outline);
	}

	outline.setOutlineColor(sf::Color::Yellow);
	for (const std::unique_ptr<WorldItem> &item : world_.getCurrentRoom()->items_) {
		const sf::FloatRect bounds = item->getBounds();
		outline.setPosition(bounds.position);
		outline.setSize(bounds.size);
		window.draw(outline);
	}
}

// Checks if player is falling off the map.
// Teleports the player back to safe position and deals 1 damage.
void GameScene::resetPlayerIfOutOfBounds()
{
	// Track last position where player was on solid ground
	if (player_.isPlayerOnGround()) {
		lastGroundPosition = player_.getPosition();
		lastPlayerDirection = player_.getDirection();
	}

	// Check if player fell off the map and apply damage once per fall
	const float worldHeight = world_.getWorldHeight();
	if (player_.getPosition().y > worldHeight) {
		if (!isPlayerFalling) {
			player_.health.damage(1);
			player_.triggerHurtFlash();

			const float safeOffsetX = (lastPlayerDirection == Direction::Left) ? 60.f : -60.f;
			const sf::Vector2f safePosition{lastGroundPosition.x + safeOffsetX, lastGroundPosition.y - 5.f};
			player_.setPosition(safePosition);
			player_.resetVelocity();

			isPlayerFalling = true;
		}
	} else {
		// Player is back in bounds
		isPlayerFalling = false;
	}
}
