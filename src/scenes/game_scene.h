#pragma once
#include "../combat/combat_system.h"
#include "../combat/hitbox.h"
#include "../core/input_manager.h"
#include "../core/scene.h"
#include "../core/scene_stack.h"
#include "../entities/player/player.h"
#include "../ui/hotbar_hud.h"
#include "../ui/minimap.h"
#include "../world/world.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

// Gameplay scene. Owns the world, player, enemies and the camera view.
class GameScene : public Scene {
  public:
	GameScene(SceneStack &sceneStack, sf::RenderWindow &window, std::string gameName, bool makeNewGame);

	void handleEvent(const sf::Event &event, sf::RenderWindow &window) override;
	void update(float deltaTime) override;
	void draw(sf::RenderWindow &window) override;

	void resetPlayerIfOutOfBounds();

  private:
	void newGame(sf::RenderWindow &window);
	void loadGame(sf::RenderWindow &window);
	void prefetchAdjacentRooms();

	void drawDebugHitboxes(sf::RenderWindow &window);
	void handleDebugInput(const InputManager &input);
	void handleHotbarInput(const InputManager &input);
	void updateEnemies(float deltaTime);
	void processEnemyEvents(); // loot, removal, boss-specific events
	void updateItems(float deltaTime);
	void resolveHitboxes();
	void updateCamera();
	void handleRoomTransition();

	SceneStack &sceneStack_;
	sf::RenderWindow &window_;
	sf::View view_;
	World world_;
	Player player_;
	CombatSystem combat_;
	HotbarHUD hotbarHud_;
	MiniMap minimap_;
	bool showMinimap_ = true;
	std::vector<Hitbox> hitboxes_;
	std::vector<Hurtbox> hurtboxes_;
	bool showDebugHitboxes_ = false;
	bool debugInvincibility_ = false;
	bool debugBuffs_ = false;
	float zoomFactor_ = 0.332188f;
	bool isPlayerFalling = false;
	sf::Vector2f lastGroundPosition{15 * 32.f, 0.f};
	Direction lastPlayerDirection;
};
