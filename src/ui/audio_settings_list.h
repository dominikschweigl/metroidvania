#pragma once
#include "slider.h"
#include "theme.h"
#include "widget.h"
#include <memory>
#include <vector>

// Widget displaying the audio volume sliders (music + sound effects)
// and wiring them to AudioManager master volumes.
class AudioSettingsList : public Widget {
  public:
	AudioSettingsList(const Theme &theme, float width);

	void handleEvent(const sf::Event &event, const sf::RenderWindow &window) override;
	void draw(sf::RenderTarget &target) const override;

	void setPosition(sf::Vector2f position) override;
	[[nodiscard]] sf::Vector2f getPosition() const override { return position_; }
	[[nodiscard]] sf::Vector2f getSize() const override;

  private:
	const Theme *theme_;
	float width_;
	sf::Vector2f position_{};

	std::vector<std::unique_ptr<Slider>> sliders_;
	int focusIndex_ = 0;
	int draggingIndex_ = -1;

	void relayout();
	void applySelection();
};
