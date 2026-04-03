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

	sf::Texture playerTexture("./assets/images/player/idle.png");
	playerTexture.setSmooth(false);
	sf::Sprite playerSprite(playerTexture);

	playerSprite.setOrigin({playerTexture.getSize().x / 2.f, static_cast<float>(playerTexture.getSize().y)});

	const float GRAVITY = 1000.f; // pixels per second squared

	sf::Clock clock;
	const float PLAYER_SPEED = 200.f;      // pixels per second
	const float JUMP_SPEED = 500.f;        // initial jump velocity
	sf::Vector2f playerPosition(0.f, 0.f); // Game state: player position
	sf::Vector2f playerVelocity(0.f, 0.f);

	playerSprite.setPosition(playerPosition);
	view.setCenter(playerPosition);
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
				// Adjust view so each world unit maps to PixelSize screen pixels,
				// filling the window with no black bars and no stretching
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

		if (playerVelocity.x != 0.f) {
			playerVelocity.x = 0.f; // Reset velocity if any key is released
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
			playerVelocity.x = -PLAYER_SPEED;
			playerDirection = Direction::Left;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
			playerVelocity.x = PLAYER_SPEED;
			playerDirection = Direction::Right;
		}

		bool isGrounded = playerPosition.y >= 0.f; // Simple grounded check

		// Always apply gravity first
		playerVelocity.y += GRAVITY * deltaTime;

		// Prevent falling below ground: if at ground and velocity is downward, zero it
		if (isGrounded) {
			playerVelocity.y = 0.f;
		}

		// Jump only when grounded and space is pressed
		if (isGrounded && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
			playerVelocity.y = -JUMP_SPEED;
		}

		// Update player position based on velocity and deltatime
		playerPosition += playerVelocity * deltaTime;
		playerSprite.setPosition(playerPosition);
		playerSprite.setScale({playerDirection == Direction::Left ? 1.f : -1.f, 1.f});

		// Update camera to follow player
		view.setCenter(playerPosition);
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

		window.draw(playerSprite);
		window.display();
	}
}
