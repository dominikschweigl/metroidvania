#include "core/scene_stack.h"
#include "scenes/game_scene.h"
#include "scenes/main_menu_scene.h"
#include <SFML/Graphics.hpp>
#include <optional>

int main()
{
	sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
	sf::RenderWindow window(desktop, "Metroidvania Game", sf::Style::Default);
	window.setFramerateLimit(60);

	SceneStack stack;
	stack.push([&stack, &window]() {
		auto newGame = [&window]() -> std::unique_ptr<Scene> { return std::make_unique<GameScene>(window.getSize()); };
		auto onExit = [&window]() { window.close(); };
		return std::make_unique<MainMenuScene>(stack, window.getSize(), std::move(newGame), std::move(onExit));
	});
	stack.applyPending();

	sf::Clock clock;
	while (window.isOpen()) {
		float deltaTime = clock.restart().asSeconds();

		while (const std::optional event = window.pollEvent()) {
			if (event->is<sf::Event::Closed>()) {
				window.close();
				break;
			}
			stack.handleEvent(*event, window);
		}

		stack.update(deltaTime);
		stack.draw(window);
		window.display();

		stack.applyPending();
		if (stack.empty())
			window.close();
	}
}
