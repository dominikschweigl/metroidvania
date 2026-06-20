#include "bluescreen_scene.h"
#include "../core/asset_manager.h"
#include <SFML/Window/Event.hpp>
#include <algorithm>

namespace {
// Input is locked out until the crash screen has been shown this long.
constexpr float kContinueDelaySeconds = 5.f;
// How long each animation frame is held before advancing to the next.
constexpr float kFrameDurationSeconds = 0.45f;
// Matches the pixel-art frames background.
const sf::Color kBackdrop{28, 52, 158};
} // namespace

BluescreenScene::BluescreenScene(sf::Vector2u windowSize, std::function<void()> onContinue)
    : windowSize_(windowSize), frames_{AssetManager::getInstance().getTexture(SEGFAULT_FRAME_0),
                                       AssetManager::getInstance().getTexture(SEGFAULT_FRAME_1),
                                       AssetManager::getInstance().getTexture(SEGFAULT_FRAME_2)},
      onContinue_(std::move(onContinue))
{
	layoutForSize(windowSize);
}

bool BluescreenScene::canContinue() const noexcept
{
	return elapsedSeconds_ >= kContinueDelaySeconds;
}

void BluescreenScene::layoutForSize(sf::Vector2u size)
{
	windowSize_ = size;
	uiView_.setSize({static_cast<float>(size.x), static_cast<float>(size.y)});
	uiView_.setCenter(uiView_.getSize() / 2.f);
}

void BluescreenScene::handleEvent(const sf::Event &event, sf::RenderWindow &window)
{
	if (const auto *resized = event.getIf<sf::Event::Resized>()) {
		layoutForSize({resized->size.x, resized->size.y});
		window.setView(uiView_);
		return;
	}
	if (!canContinue())
		return;
	if (event.is<sf::Event::KeyPressed>() || event.is<sf::Event::MouseButtonPressed>()) {
		if (onContinue_)
			onContinue_();
	}
}

void BluescreenScene::update(float deltaTime)
{
	elapsedSeconds_ += deltaTime;
}

void BluescreenScene::draw(sf::RenderWindow &window)
{
	window.setView(uiView_);
	window.clear(kBackdrop);

	const std::size_t frameIndex = static_cast<std::size_t>(elapsedSeconds_ / kFrameDurationSeconds) % frames_.size();
	const sf::Texture &texture = frames_[frameIndex];

	const sf::Vector2u textureSize = texture.getSize();
	const float scale = std::min(static_cast<float>(windowSize_.x) / static_cast<float>(textureSize.x),
	                             static_cast<float>(windowSize_.y) / static_cast<float>(textureSize.y));

	sf::Sprite sprite(texture);
	sprite.setScale({scale, scale});
	const float drawWidth = static_cast<float>(textureSize.x) * scale;
	const float drawHeight = static_cast<float>(textureSize.y) * scale;
	sprite.setPosition({(static_cast<float>(windowSize_.x) - drawWidth) / 2.f,
	                    (static_cast<float>(windowSize_.y) - drawHeight) / 2.f});
	window.draw(sprite);
}
