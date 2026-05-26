#include "game_scene.h"
#include "../core/audio_manager.h"
#include "../core/input_manager.h"
#include "menus/pause_menu.h"

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
	if (player_.hasHatThrown()) {
		player_.getThrownHat().tryHit(slime1_);
		player_.getThrownHat().tryHit(slime2_);
	}
	slime1_.update(deltaTime, world_, player_.getPosition());
	slime2_.update(deltaTime, world_, player_.getPosition());

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
	slime1_.draw(window);
	slime2_.draw(window);
}
