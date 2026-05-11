#pragma once
#include "button.h"
#include "widget.h"
#include <memory>
#include <vector>

// Vertical stack of buttons with keyboard focus management. Disabled items
// are skipped by focus traversal. Reusable for any vertical menu (main menu,
// pause menu, confirmation dialogs).
class VerticalList : public Widget {
  public:
	VerticalList(const Theme &theme, float spacing);

	void addItem(std::unique_ptr<Button> button);

	void handleEvent(const sf::Event &event, const sf::RenderWindow &window) override;
	void draw(sf::RenderTarget &target) const override;

	void setPosition(sf::Vector2f position) override;
	sf::Vector2f getPosition() const override
	{
		return position_;
	}
	sf::Vector2f getSize() const override;

  private:
	float spacing_;
	sf::Vector2f position_{};
	std::vector<std::unique_ptr<Button>> items_;
	int focusIndex_ = -1;

	void focusFirstEnabled();
	void moveFocus(int delta);
	void relayout();
	void applySelection();
};
