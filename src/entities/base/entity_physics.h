#pragma once

#include "../../world/world.h"
#include <SFML/Graphics.hpp>

namespace EntityPhysics {

// Checks if solid ground exists directly below the given bounds
bool isGroundBelow(sf::FloatRect bounds, const World &world);

// Applies gravity acceleration and updates isOnGround flag.
// Modifies velY and isOnGround by reference.
void applyGravity(float &velY, bool &isOnGround, float dt, float gravity,
				  sf::FloatRect bounds, const World &world);

// Resolves horizontal collision. Returns new x position.
// Modifies velX by reference (zeros on collision).
float resolveHorizontal(sf::Vector2f pos, float &velX, float width,
						float height, float dt, const World &world);

// Resolves vertical collision. Returns new y position.
// Modifies velY (zeros on collision) and isOnGround by reference.
float resolveVertical(sf::Vector2f pos, float &velY, bool &isOnGround,
					  float width, float height, float dt, const World &world);

// Integrates movement in fixed substeps over the given deltaTime.
// This repeats gravity and collision resolution the same way every entity does.
void simulateMovement(float deltaTime, sf::Vector2f &position,
					  sf::Vector2f &velocity, bool &isOnGround, float gravity,
					  float width, float height, const World &world);
} // namespace EntityPhysics
