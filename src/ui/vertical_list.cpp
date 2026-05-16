#include "vertical_list.h"
#include "../core/input_manager.h"

VerticalList::VerticalList(const Theme &theme, float spacing) : spacing_(spacing)
{
	(void)theme;
}

void VerticalList::addItem(std::unique_ptr<Button> button)
{
	items_.push_back(std::move(button));
	if (focusIndex_ < 0)
		focusFirstEnabled();
	relayout();
	applySelection();
}

sf::Vector2f VerticalList::getSize() const
{
	float width = 0.f;
	float height = 0.f;
	for (std::size_t i = 0; i < items_.size(); ++i) {
		const auto s = items_[i]->getSize();
		width = std::max(width, s.x);
		height += s.y;
		if (i + 1 < items_.size())
			height += spacing_;
	}
	return {width, height};
}

void VerticalList::setPosition(sf::Vector2f position)
{
	position_ = position;
	relayout();
}

void VerticalList::relayout()
{
	const float width = getSize().x;
	float y = position_.y;
	for (auto &item : items_) {
		const float itemWidth = item->getSize().x;
		item->setPosition({position_.x + (width - itemWidth) * 0.5f, y});
		y += item->getSize().y + spacing_;
	}
}

void VerticalList::focusFirstEnabled()
{
	for (std::size_t i = 0; i < items_.size(); ++i) {
		if (items_[i]->isEnabled()) {
			focusIndex_ = static_cast<int>(i);
			return;
		}
	}
	focusIndex_ = -1;
}

void VerticalList::moveFocus(int delta)
{
	if (items_.empty())
		return;
	int n = static_cast<int>(items_.size());
	int i = focusIndex_;
	for (int step = 0; step < n; ++step) {
		i = (i + delta + n) % n;
		if (items_[i]->isEnabled()) {
			focusIndex_ = i;
			applySelection();
			return;
		}
	}
}

void VerticalList::applySelection()
{
	for (int i = 0; i < static_cast<int>(items_.size()); ++i)
		items_[i]->setSelected(i == focusIndex_);
}

void VerticalList::handleEvent(const sf::Event &event, const sf::RenderWindow &window)
{
	InputManager &input = InputManager::getInstance();
	if (input.consume(MenuAction::NavigateUp)) {
		moveFocus(-1);
		return;
	}
	if (input.consume(MenuAction::NavigateDown)) {
		moveFocus(1);
		return;
	}
	if (input.consume(MenuAction::Confirm)) {
		if (focusIndex_ >= 0 && focusIndex_ < static_cast<int>(items_.size()))
			items_[focusIndex_]->activate();
		return;
	}
	// Mouse events propagate to all children
	for (auto &item : items_)
		item->handleEvent(event, window);
	applySelection();
}

void VerticalList::draw(sf::RenderTarget &target) const
{
	for (const auto &item : items_)
		item->draw(target);
}
