#include "interaction_indicator.h"
#include "../core/asset_manager.h"
#include "../core/input_manager.h"
#include <algorithm>
#include <cmath>

void InteractionIndicator::update(const float deltaTime)
{
	if (InputManager::getInstance().wasPressed(GameAction::Interact)) {
		pressTimer_ = PRESS_DURATION;
	}
	pressTimer_ = std::max(0.f, pressTimer_ - deltaTime);
}

sf::Vector2f InteractionIndicator::computeIndicatorCenter(const sf::FloatRect objectBounds, const float playerX) const
{
	const float objectCenterX = objectBounds.position.x + objectBounds.size.x / 2.f;
	const float bias = (playerX < objectCenterX ? -1.f : 1.f) * objectBounds.size.x * BIAS_FACTOR;
	const float indicatorX = objectCenterX + bias;
	return {indicatorX, objectBounds.position.y};
}

void InteractionIndicator::draw(sf::RenderWindow &window, const sf::FloatRect objectBounds, const float playerX) const
{
	const float pressDepth = (pressTimer_ > 0.f) ? DEPTH_OFFSET : 0.f;
	const sf::Vector2f center = computeIndicatorCenter(objectBounds, playerX);

	drawCapsuleOutline(window, center, pressDepth);
	drawShadowCircle(window, center);
	drawFaceCircle(window, center, pressDepth);
	drawKeyLabel(window, center, pressDepth);
}

void InteractionIndicator::drawCapsuleOutline(sf::RenderWindow &window, const sf::Vector2f center,
                                              const float pressDepth) const
{
	constexpr float PI = 3.14159265f;
	constexpr int CAPSULE_POINTS = 32;
	constexpr int HALF_POINTS = CAPSULE_POINTS / 2;
	const float borderRadius = RADIUS + BORDER_THICKNESS;

	sf::ConvexShape capsule(CAPSULE_POINTS);
	for (int i = 0; i < HALF_POINTS; ++i) {
		const float angle = PI + static_cast<float>(i) * PI / static_cast<float>(HALF_POINTS - 1);
		capsule.setPoint(
		    i, {center.x + borderRadius * std::cos(angle), center.y + pressDepth + borderRadius * std::sin(angle)});
	}
	for (int i = 0; i < HALF_POINTS; ++i) {
		const float angle = static_cast<float>(i) * PI / static_cast<float>(HALF_POINTS - 1);
		capsule.setPoint(HALF_POINTS + i, {center.x + borderRadius * std::cos(angle),
		                                   center.y + DEPTH_OFFSET + borderRadius * std::sin(angle)});
	}
	capsule.setFillColor(sf::Color(20, 20, 20));
	window.draw(capsule);
}

void InteractionIndicator::drawShadowCircle(sf::RenderWindow &window, const sf::Vector2f center) const
{
	sf::CircleShape shadow(RADIUS);
	shadow.setPosition({center.x - RADIUS, center.y - RADIUS + DEPTH_OFFSET});
	shadow.setFillColor(sf::Color(52, 52, 58));
	window.draw(shadow);
}

void InteractionIndicator::drawFaceCircle(sf::RenderWindow &window, const sf::Vector2f center,
                                          const float pressDepth) const
{
	sf::CircleShape face(RADIUS);
	face.setPosition({center.x - RADIUS, center.y - RADIUS + pressDepth});
	face.setFillColor(sf::Color(80, 80, 88));
	window.draw(face);
}

void InteractionIndicator::drawKeyLabel(sf::RenderWindow &window, const sf::Vector2f center,
                                        const float pressDepth) const
{
	const std::string keyName = InputManager::getInstance().inputName(GameAction::Interact);
	sf::Text label(AssetManager::getInstance().getFont(UI_FONT), keyName, LABEL_SIZE);
	label.setFillColor(sf::Color::White);
	const sf::FloatRect textBounds = label.getLocalBounds();
	label.setPosition({center.x - textBounds.position.x - textBounds.size.x / 2.f,
	                   center.y + pressDepth - textBounds.position.y - textBounds.size.y / 2.f});
	window.draw(label);
}
