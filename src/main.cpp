#include "core/input_manager.h"
#include "core/scene_stack.h"
#include "scenes/menus/main_menu.h"
#include <SFML/Graphics.hpp>
#include <optional>

int main()
{
	sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
	sf::RenderWindow window(desktop, "Metroidvania Game", sf::Style::Default);
	window.setFramerateLimit(60);
	window.setKeyRepeatEnabled(false);

	SceneStack stack;
	stack.push([&stack, &window]() { return makeMainMenu(stack, window); });
	stack.applyPending();

	InputManager &input_manager = InputManager::getInstance();

	sf::Clock clock;
	while (window.isOpen()) {
		float deltaTime = clock.restart().asSeconds();

		input_manager.clearFrameState();

		while (const std::optional event = window.pollEvent()) {
			if (event->is<sf::Event::Closed>()) {
				window.close();
				break;
			}
			input_manager.handleEvent(*event);
			stack.handleEvent(*event, window);
			stack.applyPending();
		}

		stack.update(deltaTime);
		stack.draw(window);
		window.display();

		stack.applyPending();
		if (stack.empty())
			window.close();
	}
}
