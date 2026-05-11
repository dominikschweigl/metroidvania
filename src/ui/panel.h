#pragma once
#include "theme.h"
#include "widget.h"
#include <memory>
#include <string>

// A solid rectangular panel with an optional title and a single child widget
// (typically a container like VerticalList). Centers its child inside the
// interior. Reused for main menu, pause menu, confirmation dialogs, etc.
class Panel : public Widget {
  public:
	Panel(const Theme &theme, sf::Vector2f size, std::string title = "");

	void setChild(std::unique_ptr<Widget> child);

	void handleEvent(const sf::Event &event, const sf::RenderWindow &window) override;
	void update(float deltaTime) override;
	void draw(sf::RenderTarget &target) const override;

	void setPosition(sf::Vector2f position) override;
	sf::Vector2f getPosition() const override
	{
		return position_;
	}
	sf::Vector2f getSize() const override
	{
		return size_;
	}

  private:
	const Theme *theme_;
	sf::Vector2f size_;
	sf::Vector2f position_{};
	std::string titleText_;
	sf::RectangleShape background_;
	sf::Text title_;
	bool hasTitle_;
	std::unique_ptr<Widget> child_;

	void layoutChild();
};
