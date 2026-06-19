#include "victory_scene.h"
#include "../core/asset_manager.h"
#include <SFML/Window/Event.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace {
constexpr float kContinueDelaySeconds = 3.5f;

const sf::Color kBackground{8, 6, 18};
const sf::Color kPanelBg{14, 11, 30};
const sf::Color kBorderColor{50, 195, 95, 210};
const sf::Color kTitleColor{220, 55, 55};
const sf::Color kGreenText{60, 215, 110};
const sf::Color kDimText{155, 160, 170};
const sf::Color kDividerColor{40, 140, 70, 100};
const sf::Color kFlavorText{185, 185, 200};
} // namespace

VictoryScene::VictoryScene(sf::Vector2u windowSize, std::function<void()> onContinue)
    : font_(AssetManager::getInstance().getFont(UI_FONT)), onContinue_(std::move(onContinue))
{
	layoutForSize(windowSize);
}

bool VictoryScene::canContinue() const noexcept
{
	return elapsedSeconds_ >= kContinueDelaySeconds;
}

void VictoryScene::layoutForSize(sf::Vector2u size)
{
	windowSize_ = size;
	uiView_.setSize({static_cast<float>(size.x), static_cast<float>(size.y)});
	uiView_.setCenter(uiView_.getSize() / 2.f);
}

void VictoryScene::handleEvent(const sf::Event &event, sf::RenderWindow &window)
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

void VictoryScene::update(float deltaTime)
{
	elapsedSeconds_ += deltaTime;
}

void VictoryScene::drawScanlines(sf::RenderWindow &window) const
{
	const float winW = static_cast<float>(windowSize_.x);
	const float winH = static_cast<float>(windowSize_.y);
	sf::VertexArray lines(sf::PrimitiveType::Lines);
	for (float y = 0.f; y < winH; y += 4.f) {
		lines.append({{0.f, y}, sf::Color{0, 0, 0, 22}});
		lines.append({{winW, y}, sf::Color{0, 0, 0, 22}});
	}
	window.draw(lines);
}

void VictoryScene::drawTextLine(sf::RenderWindow &window, const std::string &str, const unsigned int charSize,
                                const sf::Color color, const float x, float &curY, const bool centered) const
{
	sf::Text text(font_, str, charSize);
	text.setFillColor(color);
	const auto local = text.getLocalBounds();
	if (centered) {
		const float winCx = static_cast<float>(windowSize_.x) / 2.f;
		text.setPosition({winCx - local.size.x / 2.f - local.position.x, curY - local.position.y});
	} else {
		text.setPosition({x - local.position.x, curY - local.position.y});
	}
	window.draw(text);
	curY += static_cast<float>(charSize) * 1.5f;
}

void VictoryScene::draw(sf::RenderWindow &window)
{
	window.setView(uiView_);
	window.clear(kBackground);

	const float winW = static_cast<float>(windowSize_.x);
	const float winH = static_cast<float>(windowSize_.y);
	const float cx = winW / 2.f;
	const float cy = winH / 2.f;

	drawScanlines(window);

	// Moving scan bar for a CRT / terminal feel
	const float scanY = std::fmod(elapsedSeconds_ * 55.f, winH + 30.f) - 15.f;
	sf::RectangleShape scanBar({winW, 4.f});
	scanBar.setPosition({0.f, scanY});
	scanBar.setFillColor({80, 220, 130, 16});
	window.draw(scanBar);

	// Panel
	const float panelW = std::min(660.f, winW - 60.f);
	const float panelH = std::min(510.f, winH - 60.f);
	const float panelX = cx - panelW / 2.f;
	const float panelY = cy - panelH / 2.f;

	sf::RectangleShape panel({panelW, panelH});
	panel.setPosition({panelX, panelY});
	panel.setFillColor(kPanelBg);
	panel.setOutlineColor(kBorderColor);
	panel.setOutlineThickness(2.f);
	window.draw(panel);

	// Corner accents
	constexpr float kAccentSize = 10.f;
	const std::pair<float, float> corners[4] = {
	    {panelX, panelY},
	    {panelX + panelW - kAccentSize, panelY},
	    {panelX, panelY + panelH - kAccentSize},
	    {panelX + panelW - kAccentSize, panelY + panelH - kAccentSize},
	};
	for (const auto &[ax, ay] : corners) {
		sf::RectangleShape accent({kAccentSize, kAccentSize});
		accent.setPosition({ax, ay});
		accent.setFillColor(kBorderColor);
		window.draw(accent);
	}

	const float margin = 40.f;
	const float textX = panelX + margin;
	const float dividerW = panelW - margin * 2.f;
	float curY = panelY + margin;

	const auto drawDivider = [&]() {
		curY += 5.f;
		sf::RectangleShape divider({dividerW, 1.f});
		divider.setPosition({textX, curY});
		divider.setFillColor(kDividerColor);
		window.draw(divider);
		curY += 14.f;
	};

	// Title
	drawTextLine(window, "SEGMENTATION FAULT", 30, kTitleColor, textX, curY, true);
	curY += 2.f;
	drawDivider();

	// Process info block
	drawTextLine(window, "Process:    segfault_boss", 15, kDimText, textX, curY);
	drawTextLine(window, "PID:        0x5EGF4ULT", 15, kDimText, textX, curY);
	drawTextLine(window, "Signal:     SIGSEGV (11)", 15, kDimText, textX, curY);
	drawTextLine(window, "Core:       DUMPED", 15, kDimText, textX, curY);
	drawDivider();

	// Status checks
	drawTextLine(window, "[OK]  Memory corruption cleared", 15, kGreenText, textX, curY);
	drawTextLine(window, "[OK]  Forked clone terminated", 15, kGreenText, textX, curY);
	drawTextLine(window, "[OK]  NULL pointer chains severed", 15, kGreenText, textX, curY);
	drawTextLine(window, "[OK]  System integrity restored", 15, kGreenText, textX, curY);
	drawDivider();

	// Flavor text
	drawTextLine(window, "\"The memory is finally free.\"", 16, kFlavorText, textX, curY, true);
	drawDivider();

	// Prompt (pulsing once input is unlocked)
	if (canContinue()) {
		const float pulse = 0.55f + 0.45f * std::sin(elapsedSeconds_ * 3.5f);
		const auto alpha = static_cast<std::uint8_t>(pulse * 255.f);
		drawTextLine(window, "[ Press any key to continue ]", 16, {90, 210, 130, alpha}, textX, curY, true);
	}
}
