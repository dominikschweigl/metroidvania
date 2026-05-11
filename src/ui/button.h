#pragma once
#include "theme.h"
#include "widget.h"
#include <functional>
#include <string>

// A labeled, clickable/activatable button. Visual state (selected/hovered/
// disabled) is driven externally by its container or by mouse events.
class Button : public Widget {
  public:
	using Callback = std::function<void()>;

	Button(const Theme &theme, std::string label, Callback onActivate, bool enabled = true);

	void handleEvent(const sf::Event &event, const sf::RenderWindow &window) override;
	void draw(sf::RenderTarget &target) const override;

	void setPosition(sf::Vector2f position) override;
	sf::Vector2f getPosition() const override
	{
		return position_;
	}
	sf::Vector2f getSize() const override;

	void setSelected(bool selected)
	{
		selected_ = selected;
	}
	bool isEnabled() const
	{
		return enabled_;
	}

	void activate();

  private:
	const Theme *theme_;
	std::string label_;
	Callback onActivate_;
	bool enabled_;
	bool selected_ = false;
	bool hovered_ = false;
	sf::Vector2f position_{};
	mutable sf::Text text_;
	mutable sf::RectangleShape background_;

	sf::FloatRect bounds() const;
};
