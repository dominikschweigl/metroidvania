#include "panel.h"

Panel::Panel(const Theme &theme, sf::Vector2f size, std::string title)
    : theme_(&theme), size_(size), titleText_(std::move(title)), title_(theme.font, titleText_, theme.titleSize),
      hasTitle_(!titleText_.empty())
{
	background_.setSize(size_);
	background_.setFillColor(theme.panelFill);
	background_.setOutlineColor(theme.panelBorder);
	background_.setOutlineThickness(theme.panelBorderThickness);
	title_.setFillColor(theme.textNormal);
}

void Panel::setChild(std::unique_ptr<Widget> child)
{
	child_ = std::move(child);
	const sf::Vector2f contentSize = child_->getSize();
	const float titleH = hasTitle_ ? (title_.getLocalBounds().size.y + theme_->panelPadding) : 0.f;
	const float neededW = contentSize.x + theme_->panelPadding * 2.f;
	const float neededH = contentSize.y + theme_->panelPadding * 2.f + titleH;
	if (neededW > size_.x || neededH > size_.y) {
		size_.x = std::max(size_.x, neededW);
		size_.y = std::max(size_.y, neededH);
		background_.setSize(size_);
	}
	layoutChild();
}

void Panel::setPosition(sf::Vector2f position)
{
	position_ = position;
	background_.setPosition(position_);
	if (hasTitle_) {
		const auto local = title_.getLocalBounds();
		title_.setPosition({position_.x + (size_.x - local.size.x) * 0.5f - local.position.x,
		                    position_.y + theme_->panelPadding - local.position.y});
	}
	layoutChild();
}

void Panel::layoutChild()
{
	if (!child_)
		return;
	const float titleH = hasTitle_ ? (title_.getLocalBounds().size.y + theme_->panelPadding) : 0.f;
	const float interiorTop = position_.y + theme_->panelPadding + titleH;
	const float interiorHeight = size_.y - theme_->panelPadding * 2.f - titleH;
	const auto cs = child_->getSize();
	child_->setPosition(
	    {position_.x + (size_.x - cs.x) * 0.5f, interiorTop + std::max(0.f, (interiorHeight - cs.y) * 0.5f)});
}

void Panel::handleEvent(const sf::Event &event, const sf::RenderWindow &window)
{
	if (child_)
		child_->handleEvent(event, window);
}

void Panel::update(float deltaTime)
{
	if (child_)
		child_->update(deltaTime);
}

void Panel::draw(sf::RenderTarget &target) const
{
	target.draw(background_);
	if (hasTitle_)
		target.draw(title_);
	if (child_)
		child_->draw(target);
}
