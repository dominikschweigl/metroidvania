#include "button.h"

Button::Button(const Theme &theme, std::string label, Callback onActivate, bool enabled)
    : theme_(&theme), label_(std::move(label)), onActivate_(std::move(onActivate)), enabled_(enabled),
      text_(theme.font, label_, theme.itemSize)
{
	text_.setFillColor(enabled_ ? theme.textNormal : theme.textDisabled);
}

sf::Vector2f Button::getSize() const
{
	const auto local = text_.getLocalBounds();
	return {local.size.x + 2.f * theme_->itemPaddingX, local.size.y + 2.f * theme_->itemPaddingY};
}

void Button::setPosition(sf::Vector2f position)
{
	position_ = position;
	const auto local = text_.getLocalBounds();
	text_.setPosition({position.x + theme_->itemPaddingX - local.position.x,
	                   position.y + theme_->itemPaddingY - local.position.y});
}

sf::FloatRect Button::bounds() const
{
	return {position_, getSize()};
}

void Button::activate()
{
	if (enabled_ && onActivate_)
		onActivate_();
}

void Button::handleEvent(const sf::Event &event, const sf::RenderWindow &window)
{
	if (!enabled_)
		return;

	if (event.is<sf::Event::MouseMoved>()) {
		const auto mouse = sf::Mouse::getPosition(window);
		const sf::Vector2f world = window.mapPixelToCoords(mouse, window.getView());
		hovered_ = bounds().contains(world);
	} else if (const auto *click = event.getIf<sf::Event::MouseButtonPressed>()) {
		if (click->button == sf::Mouse::Button::Left) {
			const sf::Vector2f world =
			    window.mapPixelToCoords({click->position.x, click->position.y}, window.getView());
			if (bounds().contains(world))
				activate();
		}
	}
}

void Button::draw(sf::RenderTarget &target) const
{
	const bool highlight = selected_ || hovered_;
	if (highlight) {
		background_.setPosition(position_);
		background_.setSize(getSize());
		background_.setFillColor(theme_->selectionFill);
		target.draw(background_);
	}
	sf::Color color = theme_->textNormal;
	if (!enabled_)
		color = theme_->textDisabled;
	else if (highlight)
		color = theme_->textSelected;
	text_.setFillColor(color);
	target.draw(text_);
}
