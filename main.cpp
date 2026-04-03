#include <SFML/Graphics.hpp>
#include <optional>

int main()
{
	constexpr float PixelSize = 3.f; // each world "pixel" is 4x4 screen pixels

	sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
	sf::RenderWindow window(desktop, "Metroidvania Game", sf::Style::Default);

	// View that controls how many world units are visible; scaled by PixelSize
	sf::View view;
	sf::Vector2u windowSize = window.getSize();
	view.setSize({static_cast<float>(windowSize.x) / PixelSize, static_cast<float>(windowSize.y) / PixelSize});
	view.setCenter(view.getSize() / 2.f);
	window.setView(view);

	sf::Texture texture("./assets/images/player/idle.png");
	texture.setSmooth(false);
	sf::Sprite sprite(texture);

	while (window.isOpen()) {
		while (const std::optional event = window.pollEvent()) {
			if (event->is<sf::Event::Closed>()) {
				window.close();
			} else if (const auto *resized = event->getIf<sf::Event::Resized>()) {
				// Adjust view so each world unit maps to PixelSize screen pixels,
				// filling the window with no black bars and no stretching
				view.setSize(
				    {static_cast<float>(resized->size.x) / PixelSize, static_cast<float>(resized->size.y) / PixelSize});
				view.setCenter(view.getSize() / 2.f);
				window.setView(view);
			}
		}

		window.clear();
		window.draw(sprite);
		window.display();
	}
}
