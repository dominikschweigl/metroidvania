#include "segfault_boss_renderer.h"
#include "../../../../core/asset_manager.h"

#include <cmath>
#include <cstdint>

namespace segfault_boss {

namespace {
constexpr float SPEAR_PI = 3.14159265f;

// Cheap deterministic hash noise in [0, 1), used to jitter the glitch cells.
[[nodiscard]] float hashNoise(float a, float b)
{
	const float v = std::sin(a * 12.9898f + b * 78.233f) * 43758.5453f;
	return v - std::floor(v);
}
} // namespace

SegfaultBossRenderer::SegfaultBossRenderer()
    : idleTexture(AssetManager::getInstance().getTexture(SEGFAULT_BOSS_IDLE)),
      walkTexture(AssetManager::getInstance().getTexture(SEGFAULT_BOSS_WALK)),
      runTexture(AssetManager::getInstance().getTexture(SEGFAULT_BOSS_RUN)), sprite(idleTexture)
{
	sprite.setOrigin({FRAME_SIZE / 2.f, static_cast<float>(FRAME_SIZE)});
}

void SegfaultBossRenderer::setAnimation(const Animation anim, const int frame)
{
	switch (anim) {
	case Animation::Idle:
	case Animation::Death:
		sprite.setTexture(idleTexture);
		break;
	case Animation::Roaming:
		sprite.setTexture(walkTexture);
		break;
	case Animation::Attack:
		sprite.setTexture(runTexture);
		break;
	}
	const int framesPerRow = static_cast<int>(sprite.getTexture().getSize().x) / FRAME_SIZE;
	const int column = frame % framesPerRow;
	const int row = frame / framesPerRow;
	sprite.setTextureRect(sf::IntRect({column * FRAME_SIZE, row * FRAME_SIZE}, {FRAME_SIZE, FRAME_SIZE}));
}

void SegfaultBossRenderer::drawSprite(sf::RenderWindow &window, const sf::Vector2f position, const float scaleX,
                                      const sf::Color tint)
{
	sprite.setPosition(position);
	sprite.setScale({scaleX * SPRITE_SCALE, SPRITE_SCALE});
	sprite.setColor(tint);
	window.draw(sprite);
}

void SegfaultBossRenderer::drawSpearTelegraph(sf::RenderWindow &window, const sf::Vector2f footPos, const float width,
                                              const float timer) const
{
	constexpr float PIXEL = 4.f;
	constexpr float MARKER_HEIGHT = 8.f;

	const float pulse = 0.5f + 0.5f * std::sin(timer * 14.f);
	const auto snap = [](float value) { return std::round(value / PIXEL) * PIXEL; };

	sf::VertexArray cells(sf::PrimitiveType::Triangles);
	const auto addCell = [&cells](float cellX, float cellY, sf::Color color) {
		const float half = PIXEL * 0.5f;
		const sf::Vector2f topLeft{cellX - half, cellY - half};
		const sf::Vector2f topRight{cellX + half, cellY - half};
		const sf::Vector2f bottomRight{cellX + half, cellY + half};
		const sf::Vector2f bottomLeft{cellX - half, cellY + half};
		cells.append({topLeft, color});
		cells.append({topRight, color});
		cells.append({bottomRight, color});
		cells.append({topLeft, color});
		cells.append({bottomRight, color});
		cells.append({bottomLeft, color});
	};

	sf::Color color{255, 50, 50, static_cast<std::uint8_t>(90.f + 120.f * pulse)};
	for (float offsetX = -width / 2.f; offsetX <= width / 2.f; offsetX += PIXEL)
		for (float offsetY = -MARKER_HEIGHT; offsetY <= 0.f; offsetY += PIXEL)
			addCell(snap(footPos.x + offsetX), snap(footPos.y + offsetY), color);

	window.draw(cells);
}

void SegfaultBossRenderer::drawSpear(sf::RenderWindow &window, const sf::Vector2f footPos, const float width,
                                     const float height, const float timer) const
{
	constexpr float PIXEL = 4.f;

	const float flicker = std::floor(timer * 24.f); // re-rolls the glitch edge
	const auto snap = [](float value) { return std::round(value / PIXEL) * PIXEL; };

	sf::VertexArray cells(sf::PrimitiveType::Triangles);
	const auto addCell = [&cells](float cellX, float cellY, sf::Color color) {
		const float half = PIXEL * 0.5f;
		const sf::Vector2f topLeft{cellX - half, cellY - half};
		const sf::Vector2f topRight{cellX + half, cellY - half};
		const sf::Vector2f bottomRight{cellX + half, cellY + half};
		const sf::Vector2f bottomLeft{cellX - half, cellY + half};
		cells.append({topLeft, color});
		cells.append({topRight, color});
		cells.append({bottomRight, color});
		cells.append({topLeft, color});
		cells.append({bottomRight, color});
		cells.append({bottomLeft, color});
	};

	const sf::Color core{235, 250, 255};
	const sf::Color glow{120, 90, 255};

	const int rows = static_cast<int>(height / PIXEL);
	for (int row = 0; row < rows; ++row) {
		const float cellY = snap(footPos.y - row * PIXEL);
		const float up = static_cast<float>(row) / rows;

		const float taper = 1.f - up;
		const float jitter = (hashNoise(static_cast<float>(row), flicker) - 0.5f) * 2.f * PIXEL;
		const float halfWidth = std::max(PIXEL, (width / 2.f) * taper + jitter);

		for (float offsetX = -halfWidth; offsetX <= halfWidth; offsetX += PIXEL) {
			const bool edge = std::abs(offsetX) > halfWidth - PIXEL;
			sf::Color color = edge ? glow : core;
			color.a = static_cast<std::uint8_t>(180.f + 75.f * up);
			addCell(snap(footPos.x + offsetX), cellY, color);
		}
	}

	window.draw(cells);
}

} // namespace segfault_boss
