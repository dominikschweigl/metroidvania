#include "core/scene_stack.h"
#include "scenes/game_scene.h"
#include "scenes/menu_scene.h"
#include <SFML/Graphics.hpp>
#include <optional>

int main()
{
	sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
	sf::RenderWindow window(desktop, "Metroidvania Game", sf::Style::Default);
	window.setFramerateLimit(60);

	SceneStack stack;
	stack.push([&stack, &window]() {
		MenuScene::Config cfg;
		cfg.title = "Metroidvania";
		cfg.backgroundImage = "assets/images/menu_background.png";
		cfg.buttons = {
		    {"New Game",
		     [&stack, &window]() {
			     stack.replace(
			         [&window]() -> std::unique_ptr<Scene> { return std::make_unique<GameScene>(window.getSize()); });
		     }},
		    {"Load Game", {}, false},
		    {"Settings", {}, false},
		    {"Exit", [&window]() { window.close(); }},
		};
		cfg.onEscape = [&window]() { window.close(); };
		return std::make_unique<MenuScene>(window.getSize(), std::move(cfg));
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
