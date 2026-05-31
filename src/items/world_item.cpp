#include "world_item.h"
#include "../core/asset_manager.h"
#include "../entities/entity_physics.h"
#include <cmath>
#include <numbers>

WorldItem::WorldItem(const sf::Vector2f spawnPos, std::unique_ptr<Item> item)
    : item_(std::move(item)), position_(spawnPos),
      texture_(AssetManager::getInstance().getTexture(item_->textureAsset())), sprite_(texture_)
{
	sprite_.setOrigin({WIDTH / 2.f, HEIGHT});
}

void WorldItem::update(const float deltaTime, const World &world)
{
	EntityPhysics::simulateMovement(deltaTime, position_, velocity_, isOnGround_, GRAVITY, WIDTH, HEIGHT, world);

	if (isOnGround_) {
		hoverPhase_ += deltaTime * HOVER_SPEED * 2.f * std::numbers::pi_v<float>;
	}
}

void WorldItem::draw(sf::RenderWindow &window)
{
	const float visualOffsetY = isOnGround_ ? -std::sin(hoverPhase_) * HOVER_AMPLITUDE : 0.f;
	sprite_.setPosition({position_.x, position_.y + visualOffsetY});
	window.draw(sprite_);
}

sf::FloatRect WorldItem::getBounds() const noexcept
{
	return {{position_.x - WIDTH / 2.f, position_.y - HEIGHT}, {WIDTH, HEIGHT}};
}

std::unique_ptr<Item> WorldItem::tryCollect(const sf::FloatRect playerBounds)
{
	if (collected_)
		return nullptr;
	if (!playerBounds.findIntersection(getBounds()))
		return nullptr;
	collected_ = true;
	return std::move(item_);
}
