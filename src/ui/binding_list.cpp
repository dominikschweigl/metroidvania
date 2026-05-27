#include "binding_list.h"
#include <algorithm>

BindingList::BindingList(const Theme &theme, const float width, RebindHandler handler)
    : theme_(&theme), rowWidth_(width), awaitingText_(theme.font, "Press any key...", theme.itemSize),
      rebindHandler_(std::move(handler))
{
	const sf::Text sample(theme.font, "X", theme.itemSize);
	rowHeight_ = sample.getLocalBounds().size.y + 2.f * theme.itemPaddingY;

	for (const auto &meta : InputManager::gameActions()) {
		leftTexts_.emplace_back(theme.font, std::string(meta.displayName), theme.itemSize);
		refreshRightText(static_cast<int>(leftTexts_.size()) - 1);
	}
}

sf::Vector2f BindingList::getSize() const
{
	const float height = rowHeight_ * static_cast<float>(InputManager::gameActions().size());
	return {rowWidth_, height};
}

void BindingList::setPosition(const sf::Vector2f position)
{
	position_ = position;
}

void BindingList::refreshRightText(const int row)
{
	const GameAction action = InputManager::gameActions()[static_cast<std::size_t>(row)].action;
	const std::string binding = InputManager::getInstance().inputName(action);
	if (static_cast<std::size_t>(row) < rightTexts_.size()) {
		rightTexts_[row].setString(binding);
	} else {
		rightTexts_.emplace_back(theme_->font, binding, theme_->itemSize);
	}
}

void BindingList::requestRebind(const InputBinding binding)
{
	const GameAction action = InputManager::gameActions()[static_cast<std::size_t>(selectedRow_)].action;
	state_ = State::Normal;
	if (rebindHandler_) {
		rebindHandler_(action, binding);
	}
}

void BindingList::refresh()
{
	for (int i = 0; i < static_cast<int>(InputManager::gameActions().size()); ++i)
		refreshRightText(i);
}

void BindingList::handleEvent(const sf::Event &event, const sf::RenderWindow &window)
{
	InputManager &input = InputManager::getInstance();

	if (state_ == State::AwaitingInput) {
		if (input.consume(MenuAction::Back)) {
			state_ = State::Normal;
			return;
		}

		if (const auto *key = event.getIf<sf::Event::KeyPressed>()) {
			requestRebind(key->scancode);
		} else if (const auto *mouse = event.getIf<sf::Event::MouseButtonPressed>()) {
			requestRebind(mouse->button);
		}

		return;
	}

	const int rowCount = static_cast<int>(InputManager::gameActions().size());

	if (input.consume(MenuAction::NavigateUp)) {
		selectedRow_ = std::max(0, selectedRow_ - 1);
		return;
	}
	if (input.consume(MenuAction::NavigateDown)) {
		selectedRow_ = std::min(rowCount - 1, selectedRow_ + 1);
		return;
	}
	if (input.consume(MenuAction::Confirm)) {
		if (selectedRow_ >= 0 && selectedRow_ < rowCount)
			state_ = State::AwaitingInput;
		return;
	}

	if (const auto *click = event.getIf<sf::Event::MouseButtonPressed>()) {
		if (click->button == sf::Mouse::Button::Left) {
			const sf::Vector2f worldPos =
			    window.mapPixelToCoords({click->position.x, click->position.y}, window.getView());
			for (int i = 0; i < rowCount; ++i) {
				const sf::FloatRect rowBounds{{position_.x, position_.y + static_cast<float>(i) * rowHeight_},
				                              {rowWidth_, rowHeight_}};
				if (rowBounds.contains(worldPos)) {
					if (i == selectedRow_)
						state_ = State::AwaitingInput;
					else
						selectedRow_ = i;
					return;
				}
			}
		}
	}

	if (const auto *moved = event.getIf<sf::Event::MouseMoved>()) {
		const sf::Vector2f worldPos = window.mapPixelToCoords({moved->position.x, moved->position.y}, window.getView());
		for (int i = 0; i < rowCount; ++i) {
			const sf::FloatRect rowBounds{{position_.x, position_.y + static_cast<float>(i) * rowHeight_},
			                              {rowWidth_, rowHeight_}};
			if (rowBounds.contains(worldPos)) {
				selectedRow_ = i;
				return;
			}
		}
		selectedRow_ = -1;
	}
}

void BindingList::draw(sf::RenderTarget &target) const
{
	const auto actions = InputManager::gameActions();
	const float rightEdge = position_.x + rowWidth_ - theme_->itemPaddingX;

	for (std::size_t i = 0; i < actions.size(); ++i) {
		const bool isSelected = (static_cast<int>(i) == selectedRow_);
		const float rowY = position_.y + static_cast<float>(i) * rowHeight_;

		if (isSelected) {
			selectionRect_.setPosition({position_.x, rowY});
			selectionRect_.setSize({rowWidth_, rowHeight_});
			selectionRect_.setFillColor(theme_->selectionFill);
			target.draw(selectionRect_);
		}

		const sf::Color color = isSelected ? theme_->textSelected : theme_->textNormal;

		// Left text - action name
		{
			sf::Text &text = leftTexts_[i];
			text.setFillColor(color);
			const auto local = text.getLocalBounds();
			text.setPosition({position_.x + theme_->itemPaddingX - local.position.x,
			                  rowY + theme_->itemPaddingY - local.position.y});
			target.draw(text);
		}

		// Right text - binding name or "Press any key..."
		if (isSelected && state_ == State::AwaitingInput) {
			awaitingText_.setFillColor(theme_->textSelected);
			const auto local = awaitingText_.getLocalBounds();
			awaitingText_.setPosition(
			    {rightEdge - local.position.x - local.size.x, rowY + theme_->itemPaddingY - local.position.y});
			target.draw(awaitingText_);
		} else {
			sf::Text &text = rightTexts_[i];
			text.setFillColor(color);
			const auto local = text.getLocalBounds();
			text.setPosition(
			    {rightEdge - local.position.x - local.size.x, rowY + theme_->itemPaddingY - local.position.y});
			target.draw(text);
		}
	}
}
