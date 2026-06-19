#pragma once
#include "../../core/scene.h"
#include "../../core/scene_stack.h"
#include "../bluescreen_scene.h"
#include <SFML/Graphics.hpp>
#include <memory>

// Stage 2 -> 3 interrupt: a full-screen "bluescreen" that freezes the fight until
// the player dismisses it. Renders the faux Windows BSOD in BluescreenScene.
inline std::unique_ptr<Scene> makeBluescreenMenu(SceneStack &stack, sf::RenderWindow &window)
{
	return std::make_unique<BluescreenScene>(window.getSize(), [&stack]() { stack.pop(); });
}
