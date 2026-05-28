#include "game_scene.h"
#include "../core/audio_manager.h"
#include "../core/input_manager.h"
#include "../entities/enemies/race_condition_slime/race_condition_slime.h"
#include "menus/game_over_menu.h"
#include "menus/pause_menu.h"
#include <algorithm>
#include <cstdint>
#include <vector>

GameScene::GameScene(SceneStack &sceneStack, sf::RenderWindow &window) : sceneStack_(sceneStack), window_(window)
{
	AudioManager::getInstance().playMusic(MusicTrack::GAME_THEME);

	const sf::Vector2u windowSize = window.getSize();
	view_.setSize({static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)});
	view_.setCenter(view_.getSize() / 2.f);

	world_.loadRoom("start_room", "data/maps/start_room.tmj");
	world_.loadRoom("boss_room", "data/maps/boss_room.tmj");
	world_.setCurrentRoom("start_room");

	enemies_.push_back(std::make_unique<RaceConditionSlime>(sf::Vector2f{25 * 32.f, 18 * 32.f}));
	enemies_.push_back(std::make_unique<RaceConditionSlime>(sf::Vector2f{30 * 32.f, 18 * 32.f}));

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

	const bool attackTriggered = input.wasPressed(GameAction::AttackMelee);
	const bool hatThrowTriggered = input.wasPressed(GameAction::ThrowHat);

	// world_.updateCurrentRoom(deltaTime, &player_, attackTriggered, hatThrowTriggered);

	player_.update(deltaTime, world_, attackTriggered, hatThrowTriggered);

	// Update all alive enemies and remove dead ones
	for (auto &enemy : enemies_)
		enemy->update(deltaTime, world_, player_.getPosition());
	for (auto &enemy : enemies_) {
		if (!enemy->isAlive())
			combat_.clearVictim(&enemy->health);
	}
	enemies_.erase(std::remove_if(enemies_.begin(), enemies_.end(), [](const auto &e) { return !e->isAlive(); }),
	               enemies_.end());

	hitboxes_.clear();
	hurtboxes_.clear();
	player_.collectHitboxes(hitboxes_);
	player_.collectHurtboxes(hurtboxes_);
	for (auto &enemy : enemies_) {
		enemy->collectHitboxes(hitboxes_);
		enemy->collectHurtboxes(hurtboxes_);
	}

	combat_.resolve(hitboxes_, hurtboxes_);

	std::vector<std::uint32_t> endedSourceIds;
	player_.drainEndedSourceIds(endedSourceIds);
	for (auto &enemy : enemies_)
		enemy->drainEndedSourceIds(endedSourceIds);
	for (const std::uint32_t id : endedSourceIds)
		combat_.clearSource(id);

	resetPlayerIfOutOfBounds();

	// Check for player death
	if (!player_.isAlive()) {
		sceneStack_.push([&stack = sceneStack_, &window = window_]() { return makeGameOverMenu(stack, window); });
	}

	std::string touchingDoorTargetRoom = world_.getTouchingDoorTargetRoom(player_.getBounds());
	if (!touchingDoorTargetRoom.empty()) {
		world_.setCurrentRoom(touchingDoorTargetRoom);
	}

	view_.setCenter(player_.getPosition());
}

void GameScene::draw(sf::RenderWindow &window)
{
	sf::Vector2u windowSize = window.getSize();
	view_.setSize({windowSize.x * zoomFactor_, windowSize.y * zoomFactor_});
	window.setView(view_);
	window.clear({0, 0, 0});
	world_.draw(window, view_);
	player_.draw(window);
	for (auto &enemy : enemies_)
		enemy->draw(window);

	if (showDebugHitboxes_)
		drawDebugHitboxes(window);

	healthBar_.draw(window, player_.health);
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
