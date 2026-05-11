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

	// View that controls how many world units are visible; scaled by PixelSize
	sf::View view;
	sf::Vector2u windowSize = window.getSize();
	view.setSize({static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)});
	view.setCenter(view.getSize() / 2.f);

	Player player;
	World world;
	RaceConditionSlime race_condition_enemy_1({25 * 32.f, 18 * 32.f});
	RaceConditionSlime race_condition_enemy_2({30 * 32.f, 18 * 32.f});

	world.loadTileset();
	// world.loadFromTMJ("data/maps/start_room.tmj");

	world.loadRoom("start_room", "data/maps/start_room.tmj");
	world.loadRoom("boss_room", "data/maps/boss_room.tmj");

	world.setCurrentRoom("start_room");
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

		player.update(deltaTime, &world, attackTriggered);
		race_condition_enemy_1.update(deltaTime, world, player.getPosition());
		race_condition_enemy_2.update(deltaTime, world, player.getPosition());
		view.setCenter(player.getPosition());
		view.setSize({window.getSize().x * 0.4f, window.getSize().y * 0.4f});

		if (player.getPosition().x > 18 * 32.f && player.getPosition().x <= 19 * 32.f
		    && player.getPosition().y > 10 * 32.f && player.getPosition().y <= 11 * 32.f) {
			world.setCurrentRoom("boss_room");
		}

		window.setView(view);

		// Draw background: sky (above y=0) and ground (below y=0)
		// window.clear({135, 206, 235}); // Light blue sky
		window.clear({0, 0, 0}); // Light blue sky

		world.draw(window, view);

		player.draw(window);
		race_condition_enemy_1.draw(window);
		race_condition_enemy_2.draw(window);
		stack.update(deltaTime);
		stack.draw(window);
		window.display();

		stack.applyPending();
		if (stack.empty())
			window.close();
	}
}
