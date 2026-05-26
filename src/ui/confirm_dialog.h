#pragma once
#include "theme.h"
#include "vertical_list.h"
#include "widget.h"
#include <functional>
#include <memory>
#include <string>

// Small modal-style content: a centered prompt text above a Yes/No button list.
// Keyboard nav (W/S/Up/Down + Enter) is handled by the inner VerticalList.
class ConfirmDialog : public Widget {
  public:
	ConfirmDialog(const Theme &theme, std::string prompt, std::function<void()> onConfirm,
	              std::function<void()> onCancel);

	void handleEvent(const sf::Event &event, const sf::RenderWindow &window) override;
	void update(float deltaTime) override;
	void draw(sf::RenderTarget &target) const override;

	void setPosition(sf::Vector2f position) override;
	[[nodiscard]] sf::Vector2f getPosition() const override { return position_; }
	[[nodiscard]] sf::Vector2f getSize() const override;

  private:
	const Theme *theme_;
	sf::Vector2f position_{};
	float spacing_ = 16.f;
	mutable sf::Text promptText_;
	std::unique_ptr<VerticalList> buttonList_;

	void relayout();
};
