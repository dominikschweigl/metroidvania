#include "confirm_dialog.h"
#include "button.h"
#include <algorithm>

ConfirmDialog::ConfirmDialog(const Theme &theme, std::string prompt, std::function<void()> onConfirm,
                             std::function<void()> onCancel)
    : theme_(&theme), promptText_(theme.font, prompt, theme.itemSize)
{
	buttonList_ = std::make_unique<VerticalList>(theme, theme.itemSpacing);
	buttonList_->addItem(std::make_unique<Button>(theme, "Yes", std::move(onConfirm)));
	buttonList_->addItem(std::make_unique<Button>(theme, "No", std::move(onCancel)));
}

sf::Vector2f ConfirmDialog::getSize() const
{
	const auto promptBounds = promptText_.getLocalBounds();
	const auto listSize = buttonList_->getSize();
	return {std::max(promptBounds.size.x, listSize.x), promptBounds.size.y + spacing_ + listSize.y};
}

void ConfirmDialog::setPosition(const sf::Vector2f position)
{
	position_ = position;
	relayout();
}

void ConfirmDialog::relayout()
{
	const float width = getSize().x;
	const auto promptBounds = promptText_.getLocalBounds();
	promptText_.setPosition({position_.x + (width - promptBounds.size.x) * 0.5f - promptBounds.position.x,
	                         position_.y - promptBounds.position.y});

	const auto listSize = buttonList_->getSize();
	buttonList_->setPosition({position_.x + (width - listSize.x) * 0.5f, position_.y + promptBounds.size.y + spacing_});
}

void ConfirmDialog::handleEvent(const sf::Event &event, const sf::RenderWindow &window)
{
	buttonList_->handleEvent(event, window);
}

void ConfirmDialog::update(const float deltaTime)
{
	buttonList_->update(deltaTime);
}

void ConfirmDialog::draw(sf::RenderTarget &target) const
{
	promptText_.setFillColor(theme_->textNormal);
	target.draw(promptText_);
	buttonList_->draw(target);
}
