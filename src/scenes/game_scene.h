#pragma once
#include "../core/scene.h"
#include "../core/scene_stack.h"
#include "../entities/player/player.h"
#include "../entities/race_condition_slime/race_condition_slime.h"
#include "../world/world.h"
#include <SFML/Graphics.hpp>

// Gameplay scene. Owns the world, player, enemies and the camera view.
class GameScene : public Scene {
  public:
	GameScene(SceneStack &sceneStack, sf::RenderWindow &window);

	void handleEvent(const sf::Event &event, sf::RenderWindow &window) override;
	void update(float deltaTime) override;
	void draw(sf::RenderWindow &window) override;

  private:
	SceneStack &sceneStack_;
	sf::RenderWindow &window_;
	sf::View view_;
	World world_;
	Player player_;
	RaceConditionSlime slime1_;
	RaceConditionSlime slime2_;
	float zoomFactor_ = 0.4f;
};
