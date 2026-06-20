#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "core/input_manager.h"
#include "ui/interaction_indicator.h"
#include <variant>

struct InteractionIndicatorTestAccess {
	static float getPressTimer(const InteractionIndicator &ind) { return ind.pressTimer_; }
	static sf::Vector2f computeCenter(const InteractionIndicator &ind, const sf::FloatRect bounds, const float playerX)
	{
		return ind.computeIndicatorCenter(bounds, playerX);
	}
	static constexpr float PRESS_DURATION = InteractionIndicator::PRESS_DURATION;
	static constexpr float BIAS_FACTOR = InteractionIndicator::BIAS_FACTOR;
};

static sf::Event interactKeyPressed()
{
	InputManager &im = InputManager::getInstance();
	im.resetToDefaults();
	const sf::Keyboard::Scancode scancode =
	    std::get<sf::Keyboard::Scancode>(im.getPrimaryBinding(GameAction::Interact));
	return sf::Event{sf::Event::KeyPressed{sf::Keyboard::Key::Unknown, scancode, false, false}};
}

// ─── pressTimer ───────────────────────────────────────────────────────────────

TEST_CASE("InteractionIndicator - pressTimer starts at zero")
{
	InteractionIndicator indicator;
	CHECK(InteractionIndicatorTestAccess::getPressTimer(indicator) == 0.f);
}

TEST_CASE("InteractionIndicator - pressTimer set to PRESS_DURATION when interact fires")
{
	InputManager &im = InputManager::getInstance();
	im.resetToDefaults();
	im.clearFrameState();
	im.handleEvent(interactKeyPressed());

	InteractionIndicator indicator;
	indicator.update(0.f);

	CHECK(InteractionIndicatorTestAccess::getPressTimer(indicator) == InteractionIndicatorTestAccess::PRESS_DURATION);
}

TEST_CASE("InteractionIndicator - pressTimer decrements by deltaTime each frame")
{
	InputManager &im = InputManager::getInstance();
	im.resetToDefaults();
	im.clearFrameState();
	im.handleEvent(interactKeyPressed());

	InteractionIndicator indicator;
	indicator.update(0.f);
	im.clearFrameState();

	const float deltaTime = 0.05f;
	indicator.update(deltaTime);

	CHECK(InteractionIndicatorTestAccess::getPressTimer(indicator)
	      == Catch::Approx(InteractionIndicatorTestAccess::PRESS_DURATION - deltaTime));
}

TEST_CASE("InteractionIndicator - pressTimer clamps to zero")
{
	InputManager &im = InputManager::getInstance();
	im.resetToDefaults();
	im.clearFrameState();
	im.handleEvent(interactKeyPressed());

	InteractionIndicator indicator;
	indicator.update(0.f);
	im.clearFrameState();
	indicator.update(1.f);

	CHECK(InteractionIndicatorTestAccess::getPressTimer(indicator) == 0.f);
}

TEST_CASE("InteractionIndicator - pressTimer not set when interact not pressed")
{
	InputManager &im = InputManager::getInstance();
	im.resetToDefaults();
	im.clearFrameState();

	InteractionIndicator indicator;
	indicator.update(0.f);

	CHECK(InteractionIndicatorTestAccess::getPressTimer(indicator) == 0.f);
}

// ─── computeIndicatorCenter ───────────────────────────────────────────────────

TEST_CASE("InteractionIndicator - indicator X is left of object center when player is to the left")
{
	InteractionIndicator indicator;
	const sf::FloatRect bounds{{100.f, 50.f}, {40.f, 60.f}};
	const float objectCenterX = bounds.position.x + bounds.size.x / 2.f;
	const float playerX = objectCenterX - 10.f;

	const sf::Vector2f center = InteractionIndicatorTestAccess::computeCenter(indicator, bounds, playerX);

	CHECK(center.x < objectCenterX);
}

TEST_CASE("InteractionIndicator - indicator X is right of object center when player is to the right")
{
	InteractionIndicator indicator;
	const sf::FloatRect bounds{{100.f, 50.f}, {40.f, 60.f}};
	const float objectCenterX = bounds.position.x + bounds.size.x / 2.f;
	const float playerX = objectCenterX + 10.f;

	const sf::Vector2f center = InteractionIndicatorTestAccess::computeCenter(indicator, bounds, playerX);

	CHECK(center.x > objectCenterX);
}
