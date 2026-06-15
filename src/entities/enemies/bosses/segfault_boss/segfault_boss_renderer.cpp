#include "segfault_boss_renderer.h"
#include "../../../../core/asset_manager.h"

namespace segfault_boss {

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

} // namespace segfault_boss
