#pragma once
#include <SFML/Graphics.hpp>

// Visual config shared by widgets. Swap a Theme instance to reskin the UI
// without modifying widget code.
struct Theme {
	const sf::Font &font;

	sf::Color panelFill{20, 20, 30, 220};
	sf::Color panelBorder{200, 200, 220};
	float panelBorderThickness = 2.f;
	float panelPadding = 24.f;

	sf::Color textNormal{220, 220, 220};
	sf::Color textSelected{255, 220, 80};
	sf::Color textDisabled{110, 110, 110};
	sf::Color selectionFill{255, 220, 80, 40};

	unsigned int titleSize = 48;
	unsigned int itemSize = 28;
	float itemSpacing = 14.f;
	float itemPaddingX = 20.f;
	float itemPaddingY = 6.f;
};
