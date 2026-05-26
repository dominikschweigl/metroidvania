#pragma once
#include "../core/input_manager.h"
#include "theme.h"
#include "widget.h"
#include <SFML/Graphics.hpp>
#include <functional>
#include <vector>

// Widget displaying all InputManager actions and their current bindings.
class BindingList : public Widget {
  public:
	// The handler is invoked once the user picks a key/button for the focused row.
	// It is expected to perform (or veto via a confirmation prompt) the actual rebind
	// and then call refresh() so the list redraws with the new state.
	using RebindHandler = std::function<void(GameAction, InputBinding)>;

	BindingList(const Theme &theme, float width, RebindHandler handler);

	void handleEvent(const sf::Event &event, const sf::RenderWindow &window) override;
	void draw(sf::RenderTarget &target) const override;

	void setPosition(sf::Vector2f position) override;
	[[nodiscard]] sf::Vector2f getPosition() const override { return position_; }
	[[nodiscard]] sf::Vector2f getSize() const override;

	[[nodiscard]] bool isAwaitingInput() const noexcept { return state_ == State::AwaitingInput; }

	// Re-reads all bindings from InputManager and updates the displayed text.
	void refresh();

  private:
	enum class State { Normal, AwaitingInput };

	const Theme *theme_;
	float rowWidth_;
	float rowHeight_ = 0.f;
	sf::Vector2f position_{};

	State state_ = State::Normal;
	int selectedRow_ = 0;

	RebindHandler rebindHandler_;

	mutable std::vector<sf::Text> leftTexts_;
	mutable std::vector<sf::Text> rightTexts_;
	mutable sf::Text awaitingText_;
	mutable sf::RectangleShape selectionRect_;

	void requestRebind(InputBinding binding);
	void refreshRightText(int row);
};
