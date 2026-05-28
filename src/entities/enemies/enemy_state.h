#pragma once
#include <SFML/Graphics.hpp>

class BaseEnemy;
class World;

class EnemyState {
  public:
	virtual ~EnemyState() = default;

	[[nodiscard]] virtual EnemyState *update(float deltaTime, BaseEnemy &enemy, const World &world,
	                                         sf::Vector2f playerPos) = 0;
	virtual void updateAnimation(float deltaTime, BaseEnemy &enemy) = 0;
	virtual void onEnter(BaseEnemy &enemy) {}
	virtual void onExit(BaseEnemy &enemy) {}

  protected:
	// Shared animation tracking for state frame counting.
	int currentFrame = 0;
	float frameTimer = 0.f;
};
