#include "entities/player.h"
#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <optional>

int main()
{
	sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
	sf::RenderWindow window(desktop, "Metroidvania Game", sf::Style::Default);

	// View that controls how many world units are visible; scaled by PixelSize
	sf::View view;
	sf::Vector2u windowSize = window.getSize();
	view.setSize({static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)});
	view.setCenter(view.getSize() / 2.f);

	Player player;

	sf::Clock clock;

	view.setCenter(player.getPosition());
	window.setView(view);

	enum class Direction { Left, Right };
	Direction playerDirection = Direction::Left;

	while (window.isOpen()) {
		// Calculate deltatime
		float deltaTime = clock.restart().asSeconds();

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
			}
		}

		player.update(deltaTime);
		view.setCenter(player.getPosition());

		window.setView(view);

		// Draw background: sky (above y=0) and ground (below y=0)
		window.clear({135, 206, 235}); // Light blue sky

		// Calculate visible area bounds
		sf::Vector2f viewCenter = view.getCenter();
		sf::Vector2f viewSize = view.getSize();
		float left = viewCenter.x - viewSize.x / 2.f;
		float top = viewCenter.y - viewSize.y / 2.f;
		float right = viewCenter.x + viewSize.x / 2.f;
		float bottom = viewCenter.y + viewSize.y / 2.f;

		// Draw ground (below y=0) if visible
		if (bottom > 0.f) {
			sf::RectangleShape ground({right - left, bottom - 0.f});
			ground.setFillColor({60, 60, 60}); // Dark color
			ground.setPosition({left, 0.f});
			window.draw(ground);
		}

		window.draw(player.sprite);
		window.display();
	}
}
