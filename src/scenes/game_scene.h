#pragma once
#include "../core/scene.h"
#include "../entities/player/player.h"
#include "../entities/race_condition_slime/race_condition_slime.h"
#include "../world/world.h"
#include <SFML/Graphics.hpp>

// Gameplay scene. Owns the world, player, enemies and the camera view.
class GameScene : public Scene {
  public:
	GameScene(sf::Vector2u windowSize);

	void handleEvent(const sf::Event &event, sf::RenderWindow &window) override;
	void update(float deltaTime) override;
	void draw(sf::RenderWindow &window) override;

  private:
	sf::View view_;
	World world_;
	Player player_;
	RaceConditionSlime slime1_;
	RaceConditionSlime slime2_;
	bool attackTriggered_ = false;
};
