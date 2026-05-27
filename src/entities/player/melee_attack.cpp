#include "melee_attack.h"

#include "../../core/asset_manager.h"
#include "../../core/audio_manager.h"

MeleeAttack::MeleeAttack()
    : swing_texture(AssetManager::getInstance().getTexture(PLAYER_ATTACK_SWING_UPPER_BODY)),
      overhead_texture(AssetManager::getInstance().getTexture(PLAYER_ATTACK_OVERHEAD_UPPER_BODY))
{
	comboChain = {{swing_texture, 8, 0.09f}, {overhead_texture, 8, 0.09f}};
}

void MeleeAttack::reset() noexcept
{
	comboIndex = -1;
	frame = 0;
	frameTimer = 0.f;
	comboQueued = false;
}

void MeleeAttack::trigger()
{
	if (comboIndex == -1) {
		AudioManager::getInstance().playSound(SoundEffect::PLAYER_ATTACK_MELEE);
		comboIndex = 0;
		frame = 0;
		frameTimer = 0.f;
		comboQueued = false;
	} else if (comboIndex < static_cast<int>(comboChain.size()) - 1) {
		comboQueued = true;
	}
}

void MeleeAttack::update(float dt)
{
	if (comboIndex >= 0) {
		frameTimer += dt;
		const AttackDef &atk = comboChain[comboIndex];
		if (frameTimer >= atk.frameDuration) {
			frameTimer -= atk.frameDuration;
			++frame;
			if (frame >= atk.frameCount) {
				if (comboQueued && comboIndex < static_cast<int>(comboChain.size()) - 1) {
					comboQueued = false;
					++comboIndex;
					frame = 0;
					frameTimer = 0.f;
					AudioManager::getInstance().playSound(SoundEffect::PLAYER_ATTACK_MELEE);
				} else {
					comboIndex = -1;
					comboQueued = false;
				}
			}
		}
	}
}

void MeleeAttack::applyAnimation(sf::Sprite &upper, sf::Vector2f scale, sf::Vector2f pos) const
{
	const AttackDef &atk = comboChain[comboIndex];
	upper.setTexture(atk.upperTexture);
	upper.setTextureRect(sf::IntRect({frame * 32, 0}, {32, 32}));
	upper.setPosition(pos);
	upper.setScale(scale);
}
