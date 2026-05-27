#include "game_scene.h"
#include "../core/audio_manager.h"
#include "../core/input_manager.h"
#include "menus/pause_menu.h"
#include <vector>

GameScene::GameScene(SceneStack &sceneStack, sf::RenderWindow &window)
    : sceneStack_(sceneStack), window_(window), slime1_({25 * 32.f, 18 * 32.f}), slime2_({30 * 32.f, 18 * 32.f})
{
	AudioManager::getInstance().playMusic(MusicTrack::GAME_THEME);

	const sf::Vector2u windowSize = window.getSize();
	view_.setSize({static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)});
	view_.setCenter(view_.getSize() / 2.f);

	world_.loadTileset();
	world_.loadRoom("start_room", "data/maps/start_room.tmj");
	world_.loadRoom("boss_room", "data/maps/boss_room.tmj");
	world_.setCurrentRoom("start_room");

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

	const bool attackTriggered = input.wasPressed(GameAction::AttackMelee);
	const bool hatThrowTriggered = input.wasPressed(GameAction::ThrowHat);

	player_.update(deltaTime, world_, attackTriggered, hatThrowTriggered);
	if (slime1_.isAlive())
		slime1_.update(deltaTime, world_, player_.getPosition());
	if (slime2_.isAlive())
		slime2_.update(deltaTime, world_, player_.getPosition());

	std::vector<Hitbox> hitboxes;
	if (const auto melee = player_.getMeleeHitbox())
		hitboxes.push_back(*melee);
	if (player_.hasHatThrown())
		hitboxes.push_back(player_.getThrownHat().getHitbox());
	if (slime1_.isAlive())
		if (const auto slimeHit = slime1_.getHitbox())
			hitboxes.push_back(*slimeHit);
	if (slime2_.isAlive())
		if (const auto slimeHit = slime2_.getHitbox())
			hitboxes.push_back(*slimeHit);

	std::vector<Hurtbox> hurtboxes{player_.getHurtbox()};
	if (slime1_.isAlive())
		hurtboxes.push_back(slime1_.getHurtbox());
	if (slime2_.isAlive())
		hurtboxes.push_back(slime2_.getHurtbox());

	combat_.resolve(hitboxes, hurtboxes);

	// Check for player death
	if (!player_.isAlive()) {
		sceneStack_.push([&stack = sceneStack_, &window = window_]() { return makeGameOverMenu(stack, window); });
	}

	if (player_.getPosition().x > 18 * 32.f && player_.getPosition().x <= 19 * 32.f
	    && player_.getPosition().y > 10 * 32.f && player_.getPosition().y <= 11 * 32.f) {
		world_.setCurrentRoom("boss_room");
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
	if (slime1_.isAlive())
		slime1_.draw(window);
	if (slime2_.isAlive())
		slime2_.draw(window);
}
