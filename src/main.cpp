#include "core/input_manager.h"
#include "core/scene_stack.h"
#include "scenes/menus/main_menu.h"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Text.hpp>
#include <optional>

// These two flags do impact the performance more then expected. Enabling DEBUG_OVERLAY especially.
// Nevertheless are the FPS way above 400 in any case (on my machine) reaching even 1800.

// #define FRAME_RATE
// #define DEBUG_OVERLAY

int main()
{
	sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
	sf::RenderWindow window(desktop, "Metroidvania Game", sf::Style::Default);
	// window.setFramerateLimit(60);
	window.setKeyRepeatEnabled(false);

	SceneStack stack;
	stack.push([&stack, &window]() { return makeMainMenu(stack, window); });
	stack.applyPending();

	InputManager &input_manager = InputManager::getInstance();

#if defined(FRAME_RATE) || defined(DEBUG_OVERLAY)
	constexpr int SAMPLE_COUNT = 60;
	std::array<float, SAMPLE_COUNT> frameTimes{};
	float avgFrameTimes = 0;
	std::array<float, SAMPLE_COUNT> updateTimes{};
	float avgUpdateTimes = 0;
	std::array<float, SAMPLE_COUNT> drawTimes{};
	float avgDrawTimes = 0;
	int sampleIndex = 0;
#endif

	sf::Clock clock;
#if defined(FRAME_RATE) || defined(DEBUG_OVERLAY)
	sf::Clock phaseClock;
#endif
	while (window.isOpen()) {
		float deltaTime = clock.restart().asSeconds();
#if defined(FRAME_RATE) || defined(DEBUG_OVERLAY)
		frameTimes[sampleIndex] = deltaTime;
#endif

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

#if defined(FRAME_RATE) || defined(DEBUG_OVERLAY)
		phaseClock.restart();
#endif
		stack.update(deltaTime);
#if defined(FRAME_RATE) || defined(DEBUG_OVERLAY)
		updateTimes[sampleIndex] = phaseClock.restart().asSeconds();
#endif
		stack.draw(window);
#if defined(FRAME_RATE) || defined(DEBUG_OVERLAY)
		drawTimes[sampleIndex] = phaseClock.restart().asSeconds();
		sampleIndex = (sampleIndex + 1) % SAMPLE_COUNT;
		if (sampleIndex == 0) {
			auto avg = [](const auto &arr) { return std::accumulate(arr.begin(), arr.end(), 0.f) / arr.size(); };
			avgFrameTimes = avg(frameTimes);
			avgUpdateTimes = avg(updateTimes) * 1000.f;
			avgDrawTimes = avg(drawTimes) * 1000.f;
			std::cout << std::format(
			    "Averaged over the last {:} frames: FPS: {:.0f}, Frame: {:.2f}ms, Update: {:.2f}ms, Draw: {:.2f}ms",
			    SAMPLE_COUNT, 1.f / avgFrameTimes, avgFrameTimes * 1000.f, avgUpdateTimes, avgDrawTimes)
			          << std::endl;
		}
#endif
#ifdef DEBUG_OVERLAY
		const sf::View gameView = window.getView();
		window.setView(window.getDefaultView());
		const sf::Font font("/usr/share/fonts/liberation-sans-fonts/LiberationSans-Regular.ttf");
		sf::Text text(font, std::format("FPS: {:.0f}\nFrame: {:.2f}ms\nUpdate: {:.2f}ms\nDraw: {:.2f}ms",
		                                1.f / avgFrameTimes, avgFrameTimes * 1000.f, avgUpdateTimes, avgDrawTimes));
		text.setCharacterSize(14);
		text.setFillColor(sf::Color::Yellow);
		text.setPosition({8.f, 8.f});
		window.draw(text);

		window.setView(gameView);
#endif

		window.display();

		stack.applyPending();
		if (stack.empty())
			window.close();
	}
}
