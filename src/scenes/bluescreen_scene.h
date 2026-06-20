#pragma once
#include "../core/scene.h"
#include <SFML/Graphics.hpp>
#include <array>
#include <functional>

// Full-screen "segfault" crash screen shown when the Segfault boss interrupts the
// fight between stages. Can be removed with any keyboard input after a few seconds.
class BluescreenScene : public Scene {
  public:
	BluescreenScene(sf::Vector2u windowSize, std::function<void()> onContinue);

	~BluescreenScene() override = default;
	BluescreenScene(const BluescreenScene &) = delete;
	BluescreenScene &operator=(const BluescreenScene &) = delete;
	BluescreenScene(BluescreenScene &&) = delete;
	BluescreenScene &operator=(BluescreenScene &&) = delete;

	void handleEvent(const sf::Event &event, sf::RenderWindow &window) override;
	void update(float deltaTime) override;
	void draw(sf::RenderWindow &window) override;

  private:
	sf::Vector2u windowSize_;
	sf::View uiView_;
	std::array<std::reference_wrapper<const sf::Texture>, 3> frames_;
	std::function<void()> onContinue_;
	float elapsedSeconds_ = 0.f;

	[[nodiscard]] bool canContinue() const noexcept;
	void layoutForSize(sf::Vector2u size);
};
