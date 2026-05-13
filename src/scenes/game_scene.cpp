#include "game_scene.h"
#include "menus/pause_menu.h"

GameScene::GameScene(SceneStack &sceneStack, sf::Vector2u windowSize)
    : sceneStack_(sceneStack), slime1_({25 * 32.f, 18 * 32.f}), slime2_({30 * 32.f, 18 * 32.f})
{
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
		return;
	}
	if (const auto *key = event.getIf<sf::Event::KeyPressed>()) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)
		    || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl)) {
			if (key->code == sf::Keyboard::Key::Equal)
				view_.zoom(0.9f);
			else if (key->code == sf::Keyboard::Key::Hyphen)
				view_.zoom(1.1f);
		} else if (key->code == sf::Keyboard::Key::Escape) {
			sceneStack_.push([&stack = sceneStack_, &window]() { return makePauseMenu(stack, window); });
		}
	}
	if (const auto *mouse = event.getIf<sf::Event::MouseButtonPressed>()) {
		if (mouse->button == sf::Mouse::Button::Left)
			attackTriggered_ = true;
	}
}

void GameScene::update(float deltaTime)
{
	player_.update(deltaTime, &world_, attackTriggered_);
	slime1_.update(deltaTime, world_, player_.getPosition());
	slime2_.update(deltaTime, world_, player_.getPosition());

	if (player_.getPosition().x > 18 * 32.f && player_.getPosition().x <= 19 * 32.f
	    && player_.getPosition().y > 10 * 32.f && player_.getPosition().y <= 11 * 32.f) {
		world_.setCurrentRoom("boss_room");
	}

	view_.setCenter(player_.getPosition());
	attackTriggered_ = false;
}

void GameScene::draw(sf::RenderWindow &window)
{
	sf::Vector2u windowSize = window.getSize();
	view_.setSize({windowSize.x * 0.4f, windowSize.y * 0.4f});
	window.setView(view_);
	window.clear({0, 0, 0});
	world_.draw(window, view_);
	player_.draw(window);
	slime1_.draw(window);
	slime2_.draw(window);
}
