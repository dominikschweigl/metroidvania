#include "world_item.h"
#include "../core/asset_manager.h"
#include "../entities/entity_physics.h"
#include "../utils/ItemFactory.hpp"
#include <cmath>
#include <numbers>

WorldItem::WorldItem(const sf::Vector2f spawnPos, const sf::Vector2f spawnVelocity, std::unique_ptr<Item> item)
    : item_(std::move(item)), position_(spawnPos), velocity_(spawnVelocity),
      texture_(AssetManager::getInstance().getTexture(item_->textureAsset())), sprite_(texture_)
{
	sprite_.setOrigin({WIDTH / 2.f, HEIGHT});
}

void WorldItem::update(const float deltaTime, const World &world)
{
	EntityPhysics::simulateMovement(deltaTime, position_, velocity_, isOnGround_, GRAVITY, WIDTH, HEIGHT, world);

	if (isOnGround_) {
		velocity_.x *= std::exp(-WorldItem::GROUND_FRICTION * deltaTime);

		hoverPhase_ += deltaTime * HOVER_SPEED * 2.f * std::numbers::pi_v<float>;
	}

	if (time_alive < PICKUP_COOLDOWN)
		time_alive += deltaTime;
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

std::optional<std::reference_wrapper<const Item>> WorldItem::peekItem() const noexcept
{
	if (!item_)
		return std::nullopt;
	return std::cref(*item_);
}

std::unique_ptr<Item> WorldItem::tryCollect(const sf::FloatRect playerBounds)
{
	if (collected_)
		return nullptr;
	if (!playerBounds.findIntersection(getBounds()))
		return nullptr;
	if (time_alive < PICKUP_COOLDOWN) {
		return nullptr;
	}
	collected_ = true;
	return std::move(item_);
}

json WorldItem::serialize() const
{
	return {{"position", {position_.x, position_.y}},
	        {"velocity", {velocity_.x, velocity_.y}},
	        {"hoverPhase", hoverPhase_},
	        {"isOnGround", isOnGround_},
	        {"collected", collected_},
	        {"item", item_->serialize()}};
}

std::unique_ptr<WorldItem> WorldItem::deserialize(const json &j)
{
	sf::Vector2f position;
	if (j.contains("position")) {
		position.x = j["position"][0].get<float>();
		position.y = j["position"][1].get<float>();
	}

	sf::Vector2f velocity;
	if (j.contains("velocity")) {
		velocity.x = j["velocity"][0].get<float>();
		velocity.y = j["velocity"][1].get<float>();
	}

	std::unique_ptr<Item> item;
	if (j.contains("item") && !j["item"].is_null()) {
		item = ItemFactory::create(j["item"]);
	}

	std::unique_ptr<WorldItem> worldItem = std::make_unique<WorldItem>(WorldItem(position, velocity, std::move(item)));

	if (j.contains("hoverPhase")) {
		worldItem->hoverPhase_ = j["hoverPhase"].get<float>();
	}
	if (j.contains("isOnGround")) {
		worldItem->isOnGround_ = j["isOnGround"].get<bool>();
	}
	if (j.contains("collected")) {
		worldItem->collected_ = j["collected"].get<bool>();
	}

	return std::move(worldItem);
}
