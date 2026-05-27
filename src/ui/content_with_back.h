#pragma once
#include "button.h"
#include "theme.h"
#include "widget.h"
#include <functional>
#include <memory>

// Stacks a content widget above a centered "Back" button.
// The back button is mouse-clickable; keyboard users still use Esc.
class ContentWithBack : public Widget {
  public:
	ContentWithBack(const Theme &theme, std::unique_ptr<Widget> content, std::function<void()> onBack,
	                float spacing = 16.f);

	void handleEvent(const sf::Event &event, const sf::RenderWindow &window) override;
	void update(float deltaTime) override;
	void draw(sf::RenderTarget &target) const override;

	void setPosition(sf::Vector2f position) override;
	[[nodiscard]] sf::Vector2f getPosition() const override { return position_; }
	[[nodiscard]] sf::Vector2f getSize() const override;

  private:
	sf::Vector2f position_{};
	float spacing_;
	std::unique_ptr<Widget> content_;
	std::unique_ptr<Button> backButton_;

	void relayout();
};
