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
#include "menus/victory_menu.h"
#include "story_snippets.h"
#include <algorithm>
#include <cstdint>
#include <numbers>
#include <random>
#include <vector>

static std::random_device rd;
static std::mt19937 rng(rd());

std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * std::numbers::pi_v<float>);
std::uniform_real_distribution<float> speedDist(100.0f, 200.0f);

static const std::string TRANSISTOR_BOSS_ROOM_ID = "8";
static const std::string SEGFAULT_BOSS_ROOM_ID = "16";

// The laboratory area (area 2) is the room chain reached from room 7 onwards,
// distinguished in the maps by their use of the area-2 tileset.
static bool isArea2Room(const std::string &roomId)
{
	return roomId == "11" || roomId == "13" || roomId == "14" || roomId == "15" || roomId == "16";
}

static MusicTrack musicTrackForRoom(const std::string &roomId)
{
	if (roomId == TRANSISTOR_BOSS_ROOM_ID || roomId == SEGFAULT_BOSS_ROOM_ID)
		return MusicTrack::AREA_1_BOSS_THEME;
	return isArea2Room(roomId) ? MusicTrack::AREA_2_THEME : MusicTrack::AREA_1_THEME;
}

GameScene::GameScene(SceneStack &sceneStack, sf::RenderWindow &window, std::string gameName, bool makeNewGame)
    : sceneStack_(sceneStack), window_(window), world_(gameName)
{
	const sf::Vector2u windowSize = window.getSize();
	view_.setSize({static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)});
	view_.setCenter(view_.getSize() / 2.f);

	world_.registerRoom("start_room", "data/maps/start_room.tmj", {"1"});
	world_.registerRoom("1", "data/maps/1.tmj", {"start_room", "2"});
	world_.registerRoom("2", "data/maps/2.tmj", {"1", "3", "4"});
	world_.registerRoom("3", "data/maps/3.tmj", {"2", "5"});
	world_.registerRoom("4", "data/maps/4.tmj", {"2", "7", "8"});
	world_.registerRoom("5", "data/maps/5.tmj", {"3", "6", "7"});
	world_.registerRoom("6", "data/maps/6.tmj", {"5"});
	world_.registerRoom("7", "data/maps/7.tmj", {"4", "5", "11"});
	world_.registerRoom("8", "data/maps/8.tmj", {"4"});
	world_.registerRoom("11", "data/maps/11.tmj", {"7", "13"});
	world_.registerRoom("13", "data/maps/13.tmj", {"11", "14"});
	world_.registerRoom("14", "data/maps/14.tmj", {"13", "15"});
	world_.registerRoom("15", "data/maps/15.tmj", {"14", "16"});
	world_.registerRoom("16", "data/maps/16.tmj", {"15"});

	if (makeNewGame)
		this->newGame();
	else
		this->loadGame();
	storyIntroPending_ = makeNewGame;

	AudioManager::getInstance().playMusic(musicTrackForRoom(world_.getCurrentRoomId()));

	sceneStack_.push([&player = player_, &stack = sceneStack_, windowSize = window_.getSize()]() {
		return std::make_unique<InventoryScene>(player, stack, windowSize);
	});
}

void GameScene::newGame()
{
	world_.requireLoad("start_room");
	world_.setCurrentRoom("start_room");
	player_.setPosition(world_.getCurrentRoom()->playerSpawns[0]);
	view_.setCenter(player_.getPosition());
	prefetchAdjacentRooms();
}

void GameScene::loadGame()
{
	const std::string savedRoomId = world_.readSavedRoomId();
	world_.requireLoad(savedRoomId);
	world_.loadWorldData(player_);
	view_.setCenter(player_.getPosition());
	prefetchAdjacentRooms();
}

void GameScene::prefetchAdjacentRooms()
{
	for (const auto &id : world_.getAdjacentRoomIds(world_.getCurrentRoomId()))
		world_.requestLoad(id);
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
	world_.pollFutures();
	const InputManager &input = InputManager::getInstance();

	handleDebugInput(input);
	handleHotbarInput(input);

	if (input.wasPressed(GameAction::ZoomIn))
		zoomFactor_ *= 0.9f;
	if (input.wasPressed(GameAction::ZoomOut))
		zoomFactor_ *= 1.1f;
	if (input.wasPressed(MenuAction::Back))
		sceneStack_.push([&] { return makePauseMenu(sceneStack_, window_); });
	if (input.wasPressed(GameAction::OpenInventory))
		sceneStack_.push([&] { return std::make_unique<InventoryScene>(player_, sceneStack_, window_.getSize()); });
	if (input.wasPressed(GameAction::ToggleMinimap))
		showMinimap_ = !showMinimap_;
	if (input.wasPressed(GameAction::Interact))
		handleRoomTransition();

	const bool attackTriggered = input.wasPressed(GameAction::AttackMelee);
	const bool hatThrowTriggered = input.wasPressed(GameAction::ThrowHat);
	player_.update(deltaTime, world_, attackTriggered, hatThrowTriggered);

	updateEnemies(deltaTime);
	processEnemyEvents();
	updateItems(deltaTime);

	world_.update(deltaTime);

	resolveHitboxes();
	hotbarHud_.update(deltaTime);
	resetPlayerIfOutOfBounds();

	if (!player_.isAlive() && player_.inventory().hasBackup()) {
		player_.revive();
	}
	if (!player_.isAlive()) {
		sceneStack_.push([&] { return makeGameOverMenu(sceneStack_, window_); });
		pushStoryDialogue(StorySnippets::gameOver());
	}

	maybeTriggerStoryDialogue();
	updateCamera();
}

void GameScene::handleDebugInput(const InputManager &input)
{
	if (input.wasPressed(GameAction::ToggleDebugHitboxes))
		showDebugHitboxes_ = !showDebugHitboxes_;
	if (input.wasPressed(GameAction::ToggleDebugInvincibility)) {
		debugInvincibility_ = !debugInvincibility_;
		player_.setDebugInvincible(debugInvincibility_);
	}
	if (input.wasPressed(GameAction::ToggleDebugBuffs))
		debugBuffs_ = !debugBuffs_;
	if (debugBuffs_) {
		player_.addEffect(Effect::speed());
		player_.addEffect(Effect::damage());
		player_.addEffect(Effect::resistance());
		player_.addEffect(Effect::jumpBoost());
	}
}

void GameScene::handleHotbarInput(const InputManager &input)
{
	for (int i = 0; i < Inventory::HOTBAR_SIZE; ++i) {
		if (!input.wasPressed(InputManager::hotbarSlotActions()[i]))
			continue;

		hotbarHud_.flashSlot(i);
		player_.useHotbarSlot(i, &world_);
	}
}

void GameScene::updateEnemies(float deltaTime)
{
	Room *room = world_.getCurrentRoom();

	for (auto &enemy : room->enemies_)
		enemy->update(deltaTime, world_, player_.getPosition(), player_.getBounds());

	// Drain spawns (e.g. RecursionGolem splitting)
	std::vector<std::unique_ptr<BaseEnemy>> spawned;
	for (auto &enemy : room->enemies_)
		enemy->drainSpawns(spawned);
	for (auto &s : spawned)
		room->enemies_.push_back(std::move(s));
}

void GameScene::processEnemyEvents()
{
	for (auto &enemy : world_.getCurrentRoom()->enemies_) {
		if (enemy->shouldDropLoot()) {
			for (std::unique_ptr<Item> &drop : enemy->rollDrops()) {
				const sf::Vector2f initialVelocity{std::cos(angleDist(rng)) * speedDist(rng),
				                                   std::sin(angleDist(rng)) * speedDist(rng)};
				auto worldItem = std::make_unique<WorldItem>(enemy->getPosition(), initialVelocity, std::move(drop));
				world_.getCurrentRoom()->appendItem(worldItem);
			}
		}

		if (enemy->isReadyForRemoval())
			combat_.clearVictim(&enemy->health);

		if (enemy->consumeBluescreenRequest())
			sceneStack_.push([&] { return makeBluescreenMenu(sceneStack_, window_); });
		if (enemy->consumeVictoryRequest())
			sceneStack_.push([&] { return makeVictoryMenu(sceneStack_, window_); });
		if (enemy->consumeDefeatStoryRequest())
			pushStoryDialogue(StorySnippets::afterTransistorBoss());
	}
}

void GameScene::updateItems(float deltaTime)
{
	Room *room = world_.getCurrentRoom();

	for (auto &item : room->items_)
		item->update(deltaTime, world_);

	for (const std::unique_ptr<WorldItem> &worldItem : room->items_) {
		const std::optional<std::reference_wrapper<const Item>> peeked = worldItem->peekItem();
		if (!peeked || !player_.inventory().canAdd(peeked->get()))
			continue;
		std::unique_ptr<Item> collected = worldItem->tryCollect(player_.getBounds());
		if (!collected)
			continue;

		if (dynamic_cast<HatItem *>(collected.get()))
			pushStoryDialogue(StorySnippets::pickedUpHat());
		else if (dynamic_cast<ChewingGumItem *>(collected.get()))
			pushStoryDialogue(StorySnippets::pickedUpGum());

		player_.inventory().addItem(std::move(collected));
	}
}

bool GameScene::hasUsbKey() const
{
	const std::vector<Item *> items = player_.inventory().flatten();
	return std::any_of(items.begin(), items.end(),
	                    [](const Item *item) { return dynamic_cast<const UsbKeyItem *>(item) != nullptr; });
}

void GameScene::resolveHitboxes()
{
	hitboxes_.clear();
	hurtboxes_.clear();

	player_.collectHitboxes(hitboxes_);
	player_.collectHurtboxes(hurtboxes_);

	for (auto &enemy : world_.getCurrentRoom()->enemies_) {
		enemy->collectHitboxes(hitboxes_);
		enemy->collectHurtboxes(hurtboxes_);
	}

	combat_.resolve(hitboxes_, hurtboxes_);

	std::vector<std::uint32_t> endedIds;
	player_.drainEndedSourceIds(endedIds);
	for (auto &enemy : world_.getCurrentRoom()->enemies_)
		enemy->drainEndedSourceIds(endedIds);
	for (const std::uint32_t id : endedIds)
		combat_.clearSource(id);
}

void GameScene::handleRoomTransition()
{
	Door *door = world_.getTouchingDoor(player_.getBounds());
	if (door) {
		if (door->locked) {
			pushStoryDialogue(hasUsbKey() ? StorySnippets::lockedDoorWithKey() : StorySnippets::lockedDoorNoKey());
			return;
		}
		if (door->needsToClearAllEnemies && !world_.getCurrentRoom()->enemies_.empty()) {
			pushStoryDialogue(StorySnippets::roomEnemiesRemain());
			return;
		}

		world_.requireLoad(door->targetRoomId);
		world_.setCurrentRoom(door->targetRoomId);
		prefetchAdjacentRooms();
		player_.setPosition(world_.getCurrentRoom()->playerSpawns[door->targetSpawnIdx]);

		AudioManager::getInstance().playMusic(musicTrackForRoom(world_.getCurrentRoomId()));

	} else if (world_.isTouchingSavepoint(player_.getBounds())) {
		world_.saveWorldData(player_);
	}
}

void GameScene::maybeTriggerStoryDialogue()
{
	const std::string &roomId = world_.getCurrentRoomId();

	if (storyIntroPending_) {
		storyIntroPending_ = false;
		pushStoryDialogue(StorySnippets::newGameIntro());
	} else if (roomId == TRANSISTOR_BOSS_ROOM_ID && !storyBeforeTransistorShown_) {
		storyBeforeTransistorShown_ = true;
		pushStoryDialogue(StorySnippets::beforeTransistorBoss());
	} else if (roomId == SEGFAULT_BOSS_ROOM_ID && !storyBeforeSegfaultShown_) {
		storyBeforeSegfaultShown_ = true;
		pushStoryDialogue(StorySnippets::beforeSegfaultBoss());
	}
}

void GameScene::pushStoryDialogue(std::vector<DialogueLine> lines)
{
	sceneStack_.push([this, lines = std::move(lines)]() {
		return std::make_unique<DialogueScene>(sceneStack_, window_.getSize(), lines);
	});
}

void GameScene::updateCamera()
{
	const Room *room = world_.getCurrentRoom();
	const float roomW = room->width * World::TILE_SIZE;
	const float roomH = room->height * World::TILE_SIZE;

	const float halfW = view_.getSize().x / 2.f;
	const float halfH = view_.getSize().y / 2.f;

	// If the room is narrower than the viewport, center on the room instead of the player.
	const float x =
	    (roomW <= view_.getSize().x) ? roomW / 2.f : std::clamp(player_.getPosition().x, halfW, roomW - halfW);

	const float y =
	    (roomH <= view_.getSize().y) ? roomH / 2.f : std::clamp(player_.getPosition().y, halfH, roomH - halfH);

	view_.setCenter({x, y});
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
	if (showMinimap_) {
		Room *room = world_.getCurrentRoom();
		minimap_.draw(window, player_.getPosition(), *room);
	}
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
