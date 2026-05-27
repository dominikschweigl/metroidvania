#include "slider.h"
#include <algorithm>
#include <cmath>
#include <format>

namespace {
constexpr float TRACK_HEIGHT = 6.f;
constexpr float KNOB_WIDTH = 10.f;
constexpr float KNOB_HEIGHT = 18.f;
constexpr float VALUE_TEXT_WIDTH = 56.f;
constexpr float LABEL_TRACK_GAP = 16.f;
constexpr float TRACK_VALUE_GAP = 12.f;
} // namespace

Slider::Slider(const Theme &theme, std::string label, const float rowWidth, const float minValue, const float maxValue,
               const float step)
    : theme_(&theme), label_(std::move(label)), rowWidth_(rowWidth), minValue_(minValue), maxValue_(maxValue),
      step_(step), value_(minValue), labelText_(theme.font, label_, theme.itemSize),
      valueText_(theme.font, "", theme.itemSize)
{
	const sf::Text sample(theme.font, "X", theme.itemSize);
	rowHeight_ = sample.getLocalBounds().size.y + 2.f * theme.itemPaddingY;
	refreshValueText();
}

sf::Vector2f Slider::getSize() const
{
	return {rowWidth_, rowHeight_};
}

void Slider::setPosition(const sf::Vector2f position)
{
	position_ = position;
}

sf::FloatRect Slider::rowBounds() const
{
	return {position_, getSize()};
}

sf::FloatRect Slider::trackBounds() const
{
	const float naturalWidth = labelText_.getLocalBounds().size.x;
	const float labelWidth = labelColumnWidth_ > 0.f ? labelColumnWidth_ : naturalWidth;
	const float trackLeft = position_.x + theme_->itemPaddingX + labelWidth + LABEL_TRACK_GAP;
	const float trackRight = position_.x + rowWidth_ - theme_->itemPaddingX - VALUE_TEXT_WIDTH - TRACK_VALUE_GAP;
	const float trackY = position_.y + (rowHeight_ - TRACK_HEIGHT) * 0.5f;
	return {{trackLeft, trackY}, {std::max(0.f, trackRight - trackLeft), TRACK_HEIGHT}};
}

void Slider::setValue(const float value)
{
	setValueInternal(value, false);
}

void Slider::adjust(const float delta)
{
	setValueInternal(value_ + delta, true);
}

void Slider::setValueFromTrackX(const float worldX)
{
	const auto track = trackBounds();
	if (track.size.x <= 0.f)
		return;
	const float t = std::clamp((worldX - track.position.x) / track.size.x, 0.f, 1.f);
	const float raw = minValue_ + t * (maxValue_ - minValue_);
	setValueInternal(raw, true);
}

void Slider::setValueInternal(const float newValue, const bool notify)
{
	float snapped = newValue;
	if (step_ > 0.f) {
		snapped = std::round((newValue - minValue_) / step_) * step_ + minValue_;
	}
	snapped = std::clamp(snapped, minValue_, maxValue_);
	if (snapped == value_)
		return;
	value_ = snapped;
	refreshValueText();
	if (notify && onChange_) {
		onChange_(value_);
	}
}

void Slider::refreshValueText() const
{
	valueText_.setString(std::format("{}", static_cast<int>(std::round(value_))));
}

void Slider::draw(sf::RenderTarget &target) const
{
	if (selected_) {
		selectionRect_.setPosition(position_);
		selectionRect_.setSize({rowWidth_, rowHeight_});
		selectionRect_.setFillColor(theme_->selectionFill);
		target.draw(selectionRect_);
	}

	const sf::Color textColor = selected_ ? theme_->textSelected : theme_->textNormal;

	{
		const auto local = labelText_.getLocalBounds();
		labelText_.setFillColor(textColor);
		labelText_.setPosition({position_.x + theme_->itemPaddingX - local.position.x,
		                        position_.y + theme_->itemPaddingY - local.position.y});
		target.draw(labelText_);
	}

	const auto track = trackBounds();
	trackRect_.setPosition(track.position);
	trackRect_.setSize(track.size);
	trackRect_.setFillColor(theme_->textDisabled);
	target.draw(trackRect_);

	const float range = maxValue_ - minValue_;
	const float t = range > 0.f ? (value_ - minValue_) / range : 0.f;

	fillRect_.setPosition(track.position);
	fillRect_.setSize({track.size.x * t, track.size.y});
	fillRect_.setFillColor(textColor);
	target.draw(fillRect_);

	knobRect_.setSize({KNOB_WIDTH, KNOB_HEIGHT});
	knobRect_.setPosition({track.position.x + track.size.x * t - KNOB_WIDTH * 0.5f,
	                       track.position.y + (TRACK_HEIGHT - KNOB_HEIGHT) * 0.5f});
	knobRect_.setFillColor(textColor);
	target.draw(knobRect_);

	{
		const float valueRight = position_.x + rowWidth_ - theme_->itemPaddingX;
		const auto local = valueText_.getLocalBounds();
		valueText_.setFillColor(textColor);
		valueText_.setPosition(
		    {valueRight - local.position.x - local.size.x, position_.y + theme_->itemPaddingY - local.position.y});
		target.draw(valueText_);
	}
}
