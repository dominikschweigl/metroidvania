#include "audio_settings_list.h"
#include "../core/audio_manager.h"
#include "../core/input_manager.h"

namespace {
constexpr float VOLUME_MIN = 0.f;
constexpr float VOLUME_MAX = 100.f;
constexpr float VOLUME_STEP = 5.f;
constexpr float ROW_SPACING = 8.f;
} // namespace

AudioSettingsList::AudioSettingsList(const Theme &theme, const float width) : theme_(&theme), width_(width)
{
	auto musicSlider = std::make_unique<Slider>(theme, "Music", width, VOLUME_MIN, VOLUME_MAX, VOLUME_STEP);
	musicSlider->setValue(AudioManager::getInstance().musicVolume());
	musicSlider->setOnChange([](const float v) {
		auto &audio = AudioManager::getInstance();
		audio.setMusicVolume(v);
		audio.playSound(SoundEffect::VOLUME_PREVIEW, 100.f);
	});
	sliders_.push_back(std::move(musicSlider));

	auto soundSlider = std::make_unique<Slider>(theme, "Sound Effects", width, VOLUME_MIN, VOLUME_MAX, VOLUME_STEP);
	soundSlider->setValue(AudioManager::getInstance().soundVolume());
	soundSlider->setOnChange([](const float v) {
		auto &audio = AudioManager::getInstance();
		audio.setSoundVolume(v);
		audio.playSound(SoundEffect::VOLUME_PREVIEW, 100.f);
	});
	sliders_.push_back(std::move(soundSlider));

	float labelColumnWidth = 0.f;
	for (const char *label : {"Music", "Sound Effects"}) {
		const sf::Text sample(theme.font, label, theme.itemSize);
		labelColumnWidth = std::max(labelColumnWidth, sample.getLocalBounds().size.x);
	}
	for (auto &slider : sliders_) {
		slider->setLabelColumnWidth(labelColumnWidth);
	}

	applySelection();
}

sf::Vector2f AudioSettingsList::getSize() const
{
	float height = 0.f;
	for (std::size_t i = 0; i < sliders_.size(); ++i) {
		height += sliders_[i]->getSize().y;
		if (i + 1 < sliders_.size())
			height += ROW_SPACING;
	}
	return {width_, height};
}

void AudioSettingsList::setPosition(const sf::Vector2f position)
{
	position_ = position;
	relayout();
}

void AudioSettingsList::relayout()
{
	float y = position_.y;
	for (auto &slider : sliders_) {
		slider->setPosition({position_.x, y});
		y += slider->getSize().y + ROW_SPACING;
	}
}

void AudioSettingsList::applySelection()
{
	for (int i = 0; i < static_cast<int>(sliders_.size()); ++i)
		sliders_[i]->setSelected(i == focusIndex_);
}

void AudioSettingsList::handleEvent(const sf::Event &event, const sf::RenderWindow &window)
{
	InputManager &input = InputManager::getInstance();
	const int rowCount = static_cast<int>(sliders_.size());

	if (input.consume(MenuAction::NavigateUp)) {
		focusIndex_ = std::max(0, focusIndex_ - 1);
		applySelection();
		return;
	}
	if (input.consume(MenuAction::NavigateDown)) {
		focusIndex_ = std::min(rowCount - 1, focusIndex_ + 1);
		applySelection();
		return;
	}
	if (focusIndex_ >= 0 && focusIndex_ < rowCount) {
		if (input.consume(MenuAction::NavigateLeft)) {
			sliders_[focusIndex_]->adjust(-5.f);
			return;
		}
		if (input.consume(MenuAction::NavigateRight)) {
			sliders_[focusIndex_]->adjust(5.f);
			return;
		}
	}

	if (const auto *click = event.getIf<sf::Event::MouseButtonPressed>()) {
		if (click->button == sf::Mouse::Button::Left) {
			const sf::Vector2f worldPos =
			    window.mapPixelToCoords({click->position.x, click->position.y}, window.getView());
			for (int i = 0; i < rowCount; ++i) {
				if (sliders_[i]->rowBounds().contains(worldPos)) {
					focusIndex_ = i;
					applySelection();
					if (sliders_[i]->trackBounds().contains(worldPos)) {
						sliders_[i]->setValueFromTrackX(worldPos.x);
					}
					return;
				}
			}
		}
	}
}

void AudioSettingsList::draw(sf::RenderTarget &target) const
{
	for (const auto &slider : sliders_)
		slider->draw(target);
}
