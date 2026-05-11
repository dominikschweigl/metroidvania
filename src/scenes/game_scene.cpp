#include "game_scene.h"

GameScene::GameScene(sf::Vector2u windowSize)
    : slime1_({5 * 32.f, 15 * 32.f}), slime2_({11 * 32.f, 15 * 32.f})
{
	view_.setSize({static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)});
	view_.setCenter(view_.getSize() / 2.f);

	world_.loadFromTMJ("data/maps/test.tmj");
	world_.loadTileset();

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
	view_.setCenter(player_.getPosition());
	attackTriggered_ = false;
}

void GameScene::draw(sf::RenderWindow &window)
{
	window.setView(view_);
	window.clear({135, 206, 235});
	world_.draw(window, view_);
	player_.draw(window);
	slime1_.draw(window);
	slime2_.draw(window);
}
