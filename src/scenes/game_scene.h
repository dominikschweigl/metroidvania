#pragma once
#include "../combat/combat_system.h"
#include "../combat/hitbox.h"
#include "../core/scene.h"
#include "../core/scene_stack.h"
#include "../entities/enemies/base_enemy.h"
#include "../entities/player/player.h"
#include "../ui/health_bar.h"
#include "../world/world.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

// Gameplay scene. Owns the world, player, enemies and the camera view.
class GameScene : public Scene {
  public:
	GameScene(SceneStack &sceneStack, sf::RenderWindow &window);

	void handleEvent(const sf::Event &event, sf::RenderWindow &window) override;
	void update(float deltaTime) override;
	void draw(sf::RenderWindow &window) override;

	void resetPlayerIfOutOfBounds();

  private:
	void drawDebugHitboxes(sf::RenderWindow &window);

	SceneStack &sceneStack_;
	sf::RenderWindow &window_;
	sf::View view_;
	World world_;
	Player player_;
	std::vector<std::unique_ptr<BaseEnemy>> enemies_;
	CombatSystem combat_;
	HealthBar healthBar_;
	std::vector<Hitbox> hitboxes_;
	std::vector<Hurtbox> hurtboxes_;
	bool showDebugHitboxes_ = false;
	float zoomFactor_ = 0.4f;
	bool isPlayerFalling = false;
	sf::Vector2f lastGroundPosition{15 * 32.f, 0.f};
	Direction lastPlayerDirection;
};
