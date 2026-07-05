#pragma once
#include "dialogue_scene.h"
#include <vector>

// Gameplay text shown at the key parts of the game.
namespace StorySnippets {

[[nodiscard]] std::vector<DialogueLine> newGameIntro();
[[nodiscard]] std::vector<DialogueLine> beforeTransistorBoss();
[[nodiscard]] std::vector<DialogueLine> afterTransistorBoss();
[[nodiscard]] std::vector<DialogueLine> beforeSegfaultBoss();
[[nodiscard]] std::vector<DialogueLine> epilogue();
[[nodiscard]] std::vector<DialogueLine> gameOver();

[[nodiscard]] std::vector<DialogueLine> lockedDoorNoKey();
[[nodiscard]] std::vector<DialogueLine> lockedDoorWithKey();
[[nodiscard]] std::vector<DialogueLine> roomEnemiesRemain();
[[nodiscard]] std::vector<DialogueLine> pickedUpHat();
[[nodiscard]] std::vector<DialogueLine> pickedUpGum();

} // namespace StorySnippets
