#pragma once
#include "dialogue_scene.h"
#include <vector>

// Story text shown at the key parts of the game.
namespace StorySnippets {

[[nodiscard]] std::vector<DialogueLine> newGameIntro();
[[nodiscard]] std::vector<DialogueLine> beforeTransistorBoss();
[[nodiscard]] std::vector<DialogueLine> afterTransistorBoss();
[[nodiscard]] std::vector<DialogueLine> beforeSegfaultBoss();
[[nodiscard]] std::vector<DialogueLine> epilogue();
[[nodiscard]] std::vector<DialogueLine> gameOver();

} // namespace StorySnippets
