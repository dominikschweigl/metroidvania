#include "victory_scene.h"
#include "../core/asset_manager.h"
#include <SFML/Window/Event.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace {
constexpr float CONTINUE_DELAY_SECONDS = 3.5f;

const sf::Color BACKGROUND{8, 6, 18};
const sf::Color PANEL_BG{14, 11, 30};
const sf::Color BORDER_COLOR{50, 195, 95, 210};
const sf::Color VICTORY_COLOR{255, 215, 60};
const sf::Color TITLE_COLOR{220, 55, 55};
const sf::Color GREEN_TEXT{60, 215, 110};
const sf::Color DIM_TEXT{155, 160, 170};
const sf::Color DIVIDER_COLOR{40, 140, 70, 100};
const sf::Color FLAVOR_TEXT{185, 185, 200};
} // namespace

VictoryScene::VictoryScene(sf::Vector2u windowSize, std::function<void()> onContinue)
    : font_(AssetManager::getInstance().getFont(UI_FONT)), onContinue_(std::move(onContinue))
{
	layoutForSize(windowSize);
}

bool VictoryScene::canContinue() const noexcept
{
	return elapsedSeconds_ >= CONTINUE_DELAY_SECONDS;
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
	sf::VertexArray lines(sf::PrimitiveType::Lines);
	for (unsigned int row = 0; row < windowSize_.y; row += 4) {
		const float y = static_cast<float>(row);
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
	window.clear(BACKGROUND);

	const float winW = static_cast<float>(windowSize_.x);
	const float winH = static_cast<float>(windowSize_.y);
	const float cx = winW / 2.f;
	const float cy = winH / 2.f;

	drawScanlines(window);

	const float panelW = std::min(660.f, winW - 60.f);
	const float panelH = std::min(570.f, winH - 60.f);
	const float panelX = cx - panelW / 2.f;
	const float panelY = cy - panelH / 2.f;

	sf::RectangleShape panel({panelW, panelH});
	panel.setPosition({panelX, panelY});
	panel.setFillColor(PANEL_BG);
	panel.setOutlineColor(BORDER_COLOR);
	panel.setOutlineThickness(2.f);
	window.draw(panel);

	constexpr float ACCENT_SIZE = 10.f;
	const std::pair<float, float> corners[4] = {
	    {panelX, panelY},
	    {panelX + panelW - ACCENT_SIZE, panelY},
	    {panelX, panelY + panelH - ACCENT_SIZE},
	    {panelX + panelW - ACCENT_SIZE, panelY + panelH - ACCENT_SIZE},
	};
	for (const auto &[ax, ay] : corners) {
		sf::RectangleShape accent({ACCENT_SIZE, ACCENT_SIZE});
		accent.setPosition({ax, ay});
		accent.setFillColor(BORDER_COLOR);
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
		divider.setFillColor(DIVIDER_COLOR);
		window.draw(divider);
		curY += 14.f;
	};

	drawTextLine(window, "Victory!", 42, VICTORY_COLOR, textX, curY, true);
	curY += 2.f;

	drawTextLine(window, "SEGMENTATION FAULT", 26, TITLE_COLOR, textX, curY, true);
	curY += 2.f;
	drawDivider();

	drawTextLine(window, "Process:    segfault_boss", 15, DIM_TEXT, textX, curY);
	drawTextLine(window, "PID:        0x5EGF4ULT", 15, DIM_TEXT, textX, curY);
	drawTextLine(window, "Signal:     SIGSEGV (11)", 15, DIM_TEXT, textX, curY);
	drawTextLine(window, "Core:       DUMPED", 15, DIM_TEXT, textX, curY);
	drawDivider();

	drawTextLine(window, "[OK]  Memory corruption cleared", 15, GREEN_TEXT, textX, curY);
	drawTextLine(window, "[OK]  Forked clone terminated", 15, GREEN_TEXT, textX, curY);
	drawTextLine(window, "[OK]  NULL pointer chains severed", 15, GREEN_TEXT, textX, curY);
	drawTextLine(window, "[OK]  System integrity restored", 15, GREEN_TEXT, textX, curY);
	drawDivider();

	drawTextLine(window, "\"The memory is finally free.\"", 16, FLAVOR_TEXT, textX, curY, true);
	drawDivider();

	if (canContinue()) {
		const float pulse = 0.55f + 0.45f * std::sin(elapsedSeconds_ * 3.5f);
		const auto alpha = static_cast<std::uint8_t>(pulse * 255.f);
		drawTextLine(window, "[ Go back to main menu ]", 16, {90, 210, 130, alpha}, textX, curY, true);
	}
}
