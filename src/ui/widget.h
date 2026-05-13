#pragma once
#include <SFML/Graphics.hpp>

// Minimal widget interface.
// Widgets receive events and draw to a render target. Containers compose them.
class Widget {
  public:
	virtual ~Widget() = default;

	virtual void handleEvent(const sf::Event &event, const sf::RenderWindow &window) = 0;
	virtual void update(float deltaTime) { (void)deltaTime; }
	virtual void draw(sf::RenderTarget &target) const = 0;

	virtual void setPosition(sf::Vector2f position) = 0;
	virtual sf::Vector2f getPosition() const = 0;
	virtual sf::Vector2f getSize() const = 0;
};
