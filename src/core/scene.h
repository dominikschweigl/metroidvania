#pragma once
#include <SFML/Graphics.hpp>

class SceneStack;

class Scene {
  public:
	virtual ~Scene() = default;

	virtual void handleEvent(const sf::Event &event, sf::RenderWindow &window) = 0;
	virtual void update(float deltaTime) = 0;
	virtual void draw(sf::RenderWindow &window) = 0;

	// If true, scenes underneath are drawn first
	virtual bool isTransparent() const { return false; }

	// If true, scenes underneath keep receiving update()
	virtual bool updateBelow() const { return false; }
};
