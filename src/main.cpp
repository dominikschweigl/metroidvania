#include "entities/player/player.h"
#include "entities/race_condition_slime/race_condition_slime.h"
#include "world/world.h"
#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <optional>

int main()
{
	sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
	sf::RenderWindow window(desktop, "Metroidvania Game", sf::Style::Default);
	window.setFramerateLimit(60);

	// View that controls how many world units are visible; scaled by PixelSize
	sf::View view;
	sf::Vector2u windowSize = window.getSize();
	view.setSize({static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)});
	view.setCenter(view.getSize() / 2.f);

	Player player;
	World world;
	RaceConditionSlime race_condition_enemy_1({5 * 32.f, 15 * 32.f});
	RaceConditionSlime race_condition_enemy_2({11 * 32.f, 15 * 32.f});

	// world.loadFromJson("data/maps/test.json");
	world.loadFromTMJ("data/maps/test.tmj");
	world.loadTileset();

	sf::Clock clock;

	view.setCenter(player.getPosition());
	window.setView(view);

	enum class Direction { Left, Right };
	Direction playerDirection = Direction::Left;

	while (window.isOpen()) {
		// Calculate deltatime
		float deltaTime = clock.restart().asSeconds();

		bool attackTriggered = false;
		while (const std::optional event = window.pollEvent()) {
			if (event->is<sf::Event::Closed>()) {
				window.close();
			} else if (const auto *resized = event->getIf<sf::Event::Resized>()) {
				view.setSize({static_cast<float>(resized->size.x), static_cast<float>(resized->size.y)});
				view.setCenter(view.getSize() / 2.f);
				window.setView(view);
			} else if (const auto *key = event->getIf<sf::Event::KeyPressed>()) {
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)
				    || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl)) {
					if (key->code == sf::Keyboard::Key::Equal) {         // + key
						view.zoom(0.9f);                                 // Zoom in
					} else if (key->code == sf::Keyboard::Key::Hyphen) { // - key
						view.zoom(1.1f);                                 // Zoom out
					}
				}
			} else if (const auto *mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
				if (mouse->button == sf::Mouse::Button::Left)
					attackTriggered = true;
			}
		}

		player.update(deltaTime, &world, attackTriggered);
		race_condition_enemy_1.update(deltaTime, world, player.getPosition());
		race_condition_enemy_2.update(deltaTime, world, player.getPosition());
		view.setCenter(player.getPosition());

		window.setView(view);

		// Draw background: sky (above y=0) and ground (below y=0)
		window.clear({135, 206, 235}); // Light blue sky

		world.draw(window, view);

		player.draw(window);
		race_condition_enemy_1.draw(window);
		race_condition_enemy_2.draw(window);
		window.display();
	}
}
