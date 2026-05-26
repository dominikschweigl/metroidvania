#include "content_with_back.h"
#include <algorithm>

ContentWithBack::ContentWithBack(const Theme &theme, std::unique_ptr<Widget> content, std::function<void()> onBack,
                                 const float spacing)
    : spacing_(spacing), content_(std::move(content)),
      backButton_(std::make_unique<Button>(theme, "Back", std::move(onBack)))
{
}

sf::Vector2f ContentWithBack::getSize() const
{
	const sf::Vector2f cs = content_ ? content_->getSize() : sf::Vector2f{};
	const sf::Vector2f bs = backButton_->getSize();
	return {std::max(cs.x, bs.x), cs.y + spacing_ + bs.y};
}

void ContentWithBack::setPosition(const sf::Vector2f position)
{
	position_ = position;
	relayout();
}

void ContentWithBack::relayout()
{
	const float width = getSize().x;
	const sf::Vector2f cs = content_ ? content_->getSize() : sf::Vector2f{};
	if (content_) {
		content_->setPosition({position_.x + (width - cs.x) * 0.5f, position_.y});
	}
	const sf::Vector2f bs = backButton_->getSize();
	backButton_->setPosition({position_.x + (width - bs.x) * 0.5f, position_.y + cs.y + spacing_});
}

void ContentWithBack::handleEvent(const sf::Event &event, const sf::RenderWindow &window)
{
	if (content_)
		content_->handleEvent(event, window);
	backButton_->handleEvent(event, window);
}

void ContentWithBack::update(const float deltaTime)
{
	if (content_)
		content_->update(deltaTime);
	backButton_->update(deltaTime);
}

void ContentWithBack::draw(sf::RenderTarget &target) const
{
	if (content_)
		content_->draw(target);
	backButton_->draw(target);
}
