#pragma once
#include "../core/input_manager.h"
#include "theme.h"
#include "widget.h"
#include <SFML/Graphics.hpp>
#include <vector>

// Widget displaying all InputManager actions and their current bindings.
class BindingList : public Widget {
  public:
	BindingList(const Theme &theme, float width);

	void handleEvent(const sf::Event &event, const sf::RenderWindow &window) override;
	void draw(sf::RenderTarget &target) const override;

	void setPosition(sf::Vector2f position) override;
	[[nodiscard]] sf::Vector2f getPosition() const override { return position_; }
	[[nodiscard]] sf::Vector2f getSize() const override;

	[[nodiscard]] bool isAwaitingInput() const noexcept { return state_ == State::AwaitingInput; }

  private:
	enum class State { Normal, AwaitingInput };

	const Theme *theme_;
	float rowWidth_;
	float rowHeight_ = 0.f;
	sf::Vector2f position_{};

	State state_ = State::Normal;
	int selectedRow_ = 0;

	mutable std::vector<sf::Text> leftTexts_;
	mutable std::vector<sf::Text> rightTexts_;
	mutable sf::Text awaitingText_;
	mutable sf::RectangleShape selectionRect_;

	void applyRebind(InputBinding binding);
	void refreshRightText(int row);
};
