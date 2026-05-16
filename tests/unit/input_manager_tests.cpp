#include <catch2/catch_test_macros.hpp>

#include "core/input_manager.h"

struct InputManagerTestAccess {
	static constexpr std::size_t gameActionCount() { return InputManager::actionCount; }
	static constexpr std::size_t menuActionCount() { return InputManager::menuActionCount; }
};

static sf::Event keyPressed(sf::Keyboard::Scancode scancode)
{
	return sf::Event{sf::Event::KeyPressed{sf::Keyboard::Key::Unknown, scancode, false, false}};
}

static sf::Event mousePressed(sf::Mouse::Button button)
{
	return sf::Event{sf::Event::MouseButtonPressed{button, {0, 0}}};
}

// ─── wasPressed / clearFrameState ────────────────────────────────────────────

TEST_CASE("InputManager - wasPressed fires for bound key")
{
	auto &im = InputManager::getInstance();
	im.resetToDefaults();

	im.handleEvent(keyPressed(sf::Keyboard::Scancode::Space));

	CHECK(im.wasPressed(GameAction::Jump));
	CHECK_FALSE(im.wasPressed(GameAction::MoveLeft));
}

TEST_CASE("InputManager - wasPressed fires for bound mouse button")
{
	auto &im = InputManager::getInstance();
	im.resetToDefaults();

	im.handleEvent(mousePressed(sf::Mouse::Button::Left));

	CHECK(im.wasPressed(GameAction::AttackMelee));
	CHECK_FALSE(im.wasPressed(GameAction::Jump));
}

TEST_CASE("InputManager - clearFrameState resets all flags")
{
	auto &im = InputManager::getInstance();
	im.resetToDefaults();

	im.handleEvent(keyPressed(sf::Keyboard::Scancode::Space));
	im.clearFrameState();

	CHECK_FALSE(im.wasPressed(GameAction::Jump));
}

// ─── consume ─────────────────────────────────────────────────────────────────

TEST_CASE("InputManager - consume returns true then false for GameAction")
{
	auto &im = InputManager::getInstance();
	im.resetToDefaults();

	im.handleEvent(keyPressed(sf::Keyboard::Scancode::Space));

	CHECK(im.consume(GameAction::Jump));
	CHECK_FALSE(im.consume(GameAction::Jump));
}

TEST_CASE("InputManager - consume returns true then false for MenuAction")
{
	auto &im = InputManager::getInstance();
	im.resetToDefaults();

	im.handleEvent(keyPressed(sf::Keyboard::Scancode::Enter));

	CHECK(im.consume(MenuAction::Confirm));
	CHECK_FALSE(im.consume(MenuAction::Confirm));
}

// ─── secondary bindings ───────────────────────────────────────────────────────

TEST_CASE("InputManager - secondary menu binding fires action")
{
	auto &im = InputManager::getInstance();
	im.resetToDefaults();

	// Space is the secondary binding for Confirm
	im.handleEvent(keyPressed(sf::Keyboard::Scancode::Space));

	CHECK(im.wasPressed(MenuAction::Confirm));
}

TEST_CASE("InputManager - secondary menu binding NavigateUp fires on W")
{
	auto &im = InputManager::getInstance();
	im.resetToDefaults();

	im.handleEvent(keyPressed(sf::Keyboard::Scancode::W));

	CHECK(im.wasPressed(MenuAction::NavigateUp));
}

// ─── rebind ───────────────────────────────────────────────────────────────────

TEST_CASE("InputManager - rebind makes new key fire action")
{
	auto &im = InputManager::getInstance();
	im.resetToDefaults();

	im.rebind(GameAction::Jump, sf::Keyboard::Scancode::Up);
	im.handleEvent(keyPressed(sf::Keyboard::Scancode::Up));

	CHECK(im.wasPressed(GameAction::Jump));
}

TEST_CASE("InputManager - rebind makes old key no longer fire action")
{
	auto &im = InputManager::getInstance();
	im.resetToDefaults();

	im.rebind(GameAction::Jump, sf::Keyboard::Scancode::Up);
	im.handleEvent(keyPressed(sf::Keyboard::Scancode::Space));

	CHECK_FALSE(im.wasPressed(GameAction::Jump));
}

TEST_CASE("InputManager - rebind clears conflicting action's binding")
{
	auto &im = InputManager::getInstance();
	im.resetToDefaults();

	// Rebind Jump to MoveRight's scancode — MoveRight should lose its binding
	const auto moveRightScancode = sf::Keyboard::delocalize(sf::Keyboard::Key::D);
	im.rebind(GameAction::Jump, moveRightScancode);
	im.handleEvent(keyPressed(moveRightScancode));

	CHECK(im.wasPressed(GameAction::Jump));
	CHECK_FALSE(im.wasPressed(GameAction::MoveRight));
}

TEST_CASE("InputManager - displaced action does not fire on its old scancode")
{
	auto &im = InputManager::getInstance();
	im.resetToDefaults();

	const auto moveRightScancode = sf::Keyboard::delocalize(sf::Keyboard::Key::D);
	im.rebind(GameAction::Jump, moveRightScancode);
	im.clearFrameState();
	im.handleEvent(keyPressed(moveRightScancode));

	CHECK_FALSE(im.wasPressed(GameAction::MoveRight));
}

TEST_CASE("InputManager - rebind to own current binding causes no conflict")
{
	auto &im = InputManager::getInstance();
	im.resetToDefaults();

	const auto spaceScancode = sf::Keyboard::delocalize(sf::Keyboard::Key::Space);
	im.rebind(GameAction::Jump, spaceScancode); // Jump already owns Space
	im.handleEvent(keyPressed(spaceScancode));

	CHECK(im.wasPressed(GameAction::Jump));
}

TEST_CASE("InputManager - resetToDefaults restores original bindings")
{
	auto &im = InputManager::getInstance();
	im.resetToDefaults();

	im.rebind(GameAction::Jump, sf::Keyboard::Scancode::Up);
	im.resetToDefaults();
	im.handleEvent(keyPressed(sf::Keyboard::Scancode::Space));

	CHECK(im.wasPressed(GameAction::Jump));
}

// ─── gameActions metadata ─────────────────────────────────────────────────────

TEST_CASE("InputManager - gameActions returns all actions")
{
	const auto actions = InputManager::gameActions();

	CHECK(actions.size() == InputManagerTestAccess::gameActionCount());
}
