#include "dialogue_scene.h"
#include "../core/asset_manager.h"
#include "../core/input_manager.h"
#include <SFML/Window/Event.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <sstream>

namespace {
constexpr float CHARS_PER_SECOND = 45.f;
// Ignore input right after opening so a held key does not skip the first line.
constexpr float INPUT_GRACE_SECONDS = 0.15f;
constexpr float BOX_MARGIN = 14.f;
constexpr float TEXT_PADDING = 24.f;

const sf::Color OPAQUE_BACKGROUND{8, 6, 18};
const sf::Color BOX_FILL{20, 20, 30, 235};
const sf::Color BOX_BORDER{200, 200, 220};
const sf::Color SPEAKER_COLOR{255, 220, 80};
const sf::Color TEXT_COLOR{220, 220, 220};
const sf::Color HINT_COLOR{155, 160, 170};
} // namespace

DialogueScene::DialogueScene(SceneStack &stack, const sf::Vector2u windowSize, std::vector<DialogueLine> lines,
                             std::function<void()> onFinished, const bool opaque)
    : stack_(stack), font_(AssetManager::getInstance().getFont(UI_FONT)), lines_(std::move(lines)),
      onFinished_(std::move(onFinished)), opaque_(opaque)
{
	layoutForSize(windowSize);
}

void DialogueScene::layoutForSize(const sf::Vector2u size)
{
	windowSize_ = size;
	uiView_.setSize({static_cast<float>(size.x), static_cast<float>(size.y)});
	uiView_.setCenter(uiView_.getSize() / 2.f);

	const float windowWidth = static_cast<float>(size.x);
	const float windowHeight = static_cast<float>(size.y);
	boxBounds_ = {{BOX_MARGIN, windowHeight * 0.75f + BOX_MARGIN},
	              {windowWidth - 2.f * BOX_MARGIN, windowHeight * 0.25f - 2.f * BOX_MARGIN}};
	textCharSize_ = std::clamp(static_cast<unsigned int>(boxBounds_.size.y * 0.13f), 16u, 30u);
	rewrapCurrentLine();
}

void DialogueScene::rewrapCurrentLine()
{
	wrappedText_ = wrapText(lines_[lineIndex_].text, boxBounds_.size.x - 2.f * TEXT_PADDING);
}

std::string DialogueScene::wrapText(const std::string &text, const float maxWidth) const
{
	std::istringstream words(text);
	std::string word;
	std::string line;
	std::string result;
	sf::Text measure(font_, "", textCharSize_);

	while (words >> word) {
		const std::string candidate = line.empty() ? word : line + " " + word;
		measure.setString(candidate);
		if (measure.getLocalBounds().size.x > maxWidth && !line.empty()) {
			result += line + "\n";
			line = word;
		} else {
			line = candidate;
		}
	}
	result += line;
	return result;
}

std::size_t DialogueScene::visibleCharCount() const noexcept
{
	return std::min(wrappedText_.size(), static_cast<std::size_t>(revealSeconds_ * CHARS_PER_SECOND));
}

bool DialogueScene::isLineFullyRevealed() const noexcept
{
	return visibleCharCount() >= wrappedText_.size();
}

void DialogueScene::handleEvent(const sf::Event &event, sf::RenderWindow &window)
{
	if (const auto *resized = event.getIf<sf::Event::Resized>()) {
		layoutForSize({resized->size.x, resized->size.y});
		window.setView(uiView_);
		return;
	}
	if (ageSeconds_ < INPUT_GRACE_SECONDS)
		return;
	if (event.is<sf::Event::KeyPressed>() || event.is<sf::Event::MouseButtonPressed>())
		advance();
}

void DialogueScene::advance()
{
	if (!isLineFullyRevealed()) {
		revealSeconds_ = static_cast<float>(wrappedText_.size()) / CHARS_PER_SECOND;
		return;
	}
	if (lineIndex_ + 1 < lines_.size()) {
		++lineIndex_;
		revealSeconds_ = 0.f;
		rewrapCurrentLine();
		return;
	}
	close();
}

void DialogueScene::close()
{
	// The key press that dismissed the dialogue must not leak into the scene
	// below as a player action or menu-back this frame.
	InputManager::getInstance().suppressPlayerActions();
	InputManager::getInstance().suppress(MenuAction::Back);

	stack_.pop();
	if (onFinished_)
		onFinished_();
}

void DialogueScene::update(const float deltaTime)
{
	ageSeconds_ += deltaTime;
	revealSeconds_ += deltaTime;
}

void DialogueScene::draw(sf::RenderWindow &window)
{
	window.setView(uiView_);
	if (opaque_)
		window.clear(OPAQUE_BACKGROUND);

	sf::RectangleShape box(boxBounds_.size);
	box.setPosition(boxBounds_.position);
	box.setFillColor(BOX_FILL);
	box.setOutlineColor(BOX_BORDER);
	box.setOutlineThickness(2.f);
	window.draw(box);

	const float textX = boxBounds_.position.x + TEXT_PADDING;
	float textY = boxBounds_.position.y + TEXT_PADDING * 0.75f;

	const DialogueLine &line = lines_[lineIndex_];
	if (!line.speaker.empty()) {
		sf::Text speakerText(font_, line.speaker, textCharSize_);
		speakerText.setFillColor(SPEAKER_COLOR);
		speakerText.setStyle(sf::Text::Bold);
		speakerText.setPosition({textX, textY});
		window.draw(speakerText);
		textY += static_cast<float>(textCharSize_) * 1.45f;
	}

	sf::Text bodyText(font_, wrappedText_.substr(0, visibleCharCount()), textCharSize_);
	bodyText.setFillColor(TEXT_COLOR);
	bodyText.setPosition({textX, textY});
	window.draw(bodyText);

	if (isLineFullyRevealed()) {
		const float pulse = 0.55f + 0.45f * std::sin(ageSeconds_ * 3.5f);
		const auto alpha = static_cast<std::uint8_t>(pulse * 255.f);
		const bool isLastLine = lineIndex_ + 1 == lines_.size();
		sf::Text hintText(font_, isLastLine ? "[ Close ]" : "[ Continue ]", textCharSize_ * 3u / 4u);
		hintText.setFillColor({HINT_COLOR.r, HINT_COLOR.g, HINT_COLOR.b, alpha});
		const sf::FloatRect hintBounds = hintText.getLocalBounds();
		hintText.setPosition({boxBounds_.position.x + boxBounds_.size.x - hintBounds.size.x - TEXT_PADDING,
		                      boxBounds_.position.y + boxBounds_.size.y - hintBounds.size.y - TEXT_PADDING * 0.75f});
		window.draw(hintText);
	}
}
