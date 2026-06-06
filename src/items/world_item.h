#pragma once
#include "item.h"
#include <SFML/Graphics.hpp>
#include <memory>

class World;

class WorldItem {
  public:
	static constexpr float WIDTH = 16.f;
	static constexpr float HEIGHT = 16.f;
	static constexpr float GRAVITY = 1200.f;
	static constexpr float HOVER_AMPLITUDE = 2.f;
	static constexpr float HOVER_SPEED = 0.35f;

	WorldItem(sf::Vector2f spawnPos, std::unique_ptr<Item> item);
	~WorldItem() = default;

	WorldItem(const WorldItem &) = delete;
	WorldItem &operator=(const WorldItem &) = delete;
	WorldItem(WorldItem &&) = default;
	WorldItem &operator=(WorldItem &&) = default;

	void update(float deltaTime, const World &world);
	void draw(sf::RenderWindow &window);

	// Returns the item and transfers ownership on first intersection; nullptr otherwise.
	[[nodiscard]] std::unique_ptr<Item> tryCollect(sf::FloatRect playerBounds);
	[[nodiscard]] bool isCollected() const noexcept { return collected_; }
	[[nodiscard]] sf::FloatRect getBounds() const noexcept;
	virtual json serialize() const;
	static std::unique_ptr<WorldItem> deserialize(const json &j);

  private:
	std::unique_ptr<Item> item_;
	sf::Vector2f position_;
	sf::Vector2f velocity_{0.f, 0.f};
	bool isOnGround_ = false;
	float hoverPhase_ = 0.f;
	bool collected_ = false;
	const sf::Texture &texture_;
	sf::Sprite sprite_;
};
