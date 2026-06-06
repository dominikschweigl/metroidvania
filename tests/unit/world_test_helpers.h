#pragma once
#include "world/world.h"
#include <vector>

inline World makeEmptyWorld()
{
	std::vector<std::vector<int>> g(5, std::vector<int>(10, 0));
	World w = World("test");
	w.loadFromGrid(g);
	return w;
}

inline World makeFlooredWorldForPlayer()
{
	// Player spawns at (15*32, 0). Build a wide world with a floor row directly beneath
	// so the player stays in idle (canAttack=true) after the first update tick.
	std::vector<std::vector<int>> g(20, std::vector<int>(40, 0));
	for (int x = 0; x < 40; ++x)
		g[1][x] = 1;
	World w = World("test");
	w.loadFromGrid(g);
	return w;
}
