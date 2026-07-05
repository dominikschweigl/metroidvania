#include <catch2/catch_test_macros.hpp>

#include "core/scene_stack.h"
#include "scenes/dialogue_scene.h"
#include "scenes/story_snippets.h"

namespace {

sf::Event keyPressedEvent()
{
	return sf::Event{sf::Event::KeyPressed{sf::Keyboard::Key::Enter, sf::Keyboard::Scancode::Enter, false, false}};
}

// Long enough to pass the input grace period and fully reveal any line.
constexpr float LONG_UPDATE_SECONDS = 60.f;

} // namespace

TEST_CASE("DialogueScene advances through lines and pops itself", "[dialogue_scene]")
{
	SceneStack stack;
	sf::RenderWindow window; // unopened; only needed to satisfy handleEvent's signature
	bool finished = false;

	std::vector<DialogueLine> lines = {{"You", "First line."}, {"You", "Second line."}};
	stack.push([&stack, &lines, &finished]() {
		return std::make_unique<DialogueScene>(stack, sf::Vector2u{1280, 720}, lines,
		                                       [&finished]() { finished = true; });
	});
	stack.applyPending();
	REQUIRE_FALSE(stack.empty());

	// First press on a fully revealed line advances to the second line.
	stack.update(LONG_UPDATE_SECONDS);
	stack.handleEvent(keyPressedEvent(), window);
	stack.applyPending();
	REQUIRE_FALSE(stack.empty());
	REQUIRE_FALSE(finished);

	// Dismissing the last line pops the scene and calls onFinished.
	stack.update(LONG_UPDATE_SECONDS);
	stack.handleEvent(keyPressedEvent(), window);
	REQUIRE(finished);
	stack.applyPending();
	REQUIRE(stack.empty());
}

TEST_CASE("DialogueScene reveals text gradually before advancing", "[dialogue_scene]")
{
	SceneStack stack;
	sf::RenderWindow window;
	bool finished = false;

	std::vector<DialogueLine> lines = {{"You", "A single reasonably long line of story text."}};
	stack.push([&stack, &lines, &finished]() {
		return std::make_unique<DialogueScene>(stack, sf::Vector2u{1280, 720}, lines,
		                                       [&finished]() { finished = true; });
	});
	stack.applyPending();

	// Shortly after opening the line is still being typed out, so the first
	// press only completes the reveal instead of closing the dialogue.
	stack.update(0.2f);
	stack.handleEvent(keyPressedEvent(), window);
	stack.applyPending();
	REQUIRE_FALSE(stack.empty());
	REQUIRE_FALSE(finished);

	// The next press dismisses the now fully revealed line.
	stack.handleEvent(keyPressedEvent(), window);
	REQUIRE(finished);
	stack.applyPending();
	REQUIRE(stack.empty());
}

TEST_CASE("Story snippets provide text for every story beat", "[dialogue_scene]")
{
	REQUIRE_FALSE(StorySnippets::newGameIntro().empty());
	REQUIRE_FALSE(StorySnippets::beforeTransistorBoss().empty());
	REQUIRE_FALSE(StorySnippets::afterTransistorBoss().empty());
	REQUIRE_FALSE(StorySnippets::beforeSegfaultBoss().empty());
	REQUIRE_FALSE(StorySnippets::epilogue().empty());
	REQUIRE_FALSE(StorySnippets::gameOver().empty());
}
