#pragma once
#include "theme.h"
#include "widget.h"
#include <functional>
#include <string>

// Horizontal slider row: label on the left, track + knob in the middle,
// numeric value on the right. Range [minValue, maxValue] in steps of step.
// Focus and keyboard navigation are managed by the parent container.
class Slider : public Widget {
  public:
	using OnChange = std::function<void(float)>;

	Slider(const Theme &theme, std::string label, float rowWidth, float minValue, float maxValue, float step);

	void setValue(float value);
	[[nodiscard]] float value() const noexcept { return value_; }

	void setSelected(bool selected) noexcept { selected_ = selected; }
	void setOnChange(OnChange callback) { onChange_ = std::move(callback); }
	void setLabelColumnWidth(float width) noexcept { labelColumnWidth_ = width; }

	void adjust(float delta);
	void setValueFromTrackX(float worldX);

	[[nodiscard]] sf::FloatRect rowBounds() const;
	[[nodiscard]] sf::FloatRect trackBounds() const;

	void handleEvent(const sf::Event & /*event*/, const sf::RenderWindow & /*window*/) override {}
	void draw(sf::RenderTarget &target) const override;

	void setPosition(sf::Vector2f position) override;
	[[nodiscard]] sf::Vector2f getPosition() const override { return position_; }
	[[nodiscard]] sf::Vector2f getSize() const override;

  private:
	const Theme *theme_;
	std::string label_;
	float rowWidth_;
	float minValue_;
	float maxValue_;
	float step_;
	float value_;
	bool selected_ = false;
	sf::Vector2f position_{};
	float rowHeight_;
	float labelColumnWidth_ = -1.f;

	OnChange onChange_;

	mutable sf::Text labelText_;
	mutable sf::Text valueText_;
	mutable sf::RectangleShape selectionRect_;
	mutable sf::RectangleShape trackRect_;
	mutable sf::RectangleShape fillRect_;
	mutable sf::RectangleShape knobRect_;

	void setValueInternal(float newValue, bool notify);
	void refreshValueText() const;
};
