#pragma once
#include "../core/scene.h"
#include "../core/scene_stack.h"
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>
#include <vector>

struct DialogueLine {
	std::string speaker;
	std::string text;
};

// Story text box for dialogue options during the game.
// Shown as an overlay to the current scene.
class DialogueScene : public Scene {
  public:
	DialogueScene(SceneStack &stack, sf::Vector2u windowSize, std::vector<DialogueLine> lines,
	              std::function<void()> onFinished = {}, bool opaque = false);

	~DialogueScene() override = default;
	DialogueScene(const DialogueScene &) = delete;
	DialogueScene &operator=(const DialogueScene &) = delete;
	DialogueScene(DialogueScene &&) = delete;
	DialogueScene &operator=(DialogueScene &&) = delete;

	void handleEvent(const sf::Event &event, sf::RenderWindow &window) override;
	void update(float deltaTime) override;
	void draw(sf::RenderWindow &window) override;

	bool isTransparent() const override { return !opaque_; }

  private:
	void layoutForSize(sf::Vector2u size);
	void rewrapCurrentLine();
	void advance();
	void close();

	[[nodiscard]] std::size_t visibleCharCount() const noexcept;
	[[nodiscard]] bool isLineFullyRevealed() const noexcept;
	[[nodiscard]] std::string wrapText(const std::string &text, float maxWidth) const;

	SceneStack &stack_;
	const sf::Font &font_;
	std::vector<DialogueLine> lines_;
	std::function<void()> onFinished_;
	bool opaque_;

	sf::Vector2u windowSize_;
	sf::View uiView_;
	sf::FloatRect boxBounds_;
	unsigned int textCharSize_ = 20;

	std::size_t lineIndex_ = 0;
	std::string wrappedText_;
	float revealSeconds_ = 0.f;
	float ageSeconds_ = 0.f;
};
