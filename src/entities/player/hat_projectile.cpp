#include "hat_projectile.h"
#include <cmath>

HatProjectile::HatProjectile(const sf::Vector2f startPos, const Direction direction, const sf::Vector2f playerVelocity,
                             const sf::Texture &texture)
    : startPos(startPos), pos(startPos), velocity({static_cast<float>(direction) * HAT_SPEED + playerVelocity.x, 0.f}),
      maxTravel(BASE_TRAVEL + std::abs(playerVelocity.x) * PLAYER_VELOCITY_TRAVEL_INCREASE_FACTOR), sprite(texture)
{
	sprite.setOrigin({FRAME_SIZE / 2.f, FRAME_SIZE / 2.f});
	sprite.setTextureRect(sf::IntRect({0, 0}, {FRAME_SIZE, FRAME_SIZE}));
}

bool HatProjectile::update(const float dt, const sf::Vector2f playerPos, const World &world)
{
	spinTimer += dt;
	if (spinTimer >= SPIN_FRAME_DUR) {
		spinTimer -= SPIN_FRAME_DUR;
		spinFrame = (spinFrame + 1) % SPIN_FRAME_COUNT;
		sprite.setTextureRect(sf::IntRect({spinFrame * FRAME_SIZE, 0}, {FRAME_SIZE, FRAME_SIZE}));
	}

	if (phase == Phase::Flying) {
		const float travelRatio = (maxTravel - distanceTraveled) / maxTravel;
		const float speedFactor = 1.f - std::pow(1.f - travelRatio, 4.f);
		const sf::Vector2f delta = velocity * dt * speedFactor;
		pos += delta;
		distanceTraveled += std::abs(delta.x) + std::abs(delta.y);

		const bool hitWall = world.isSolidAtRect(getBounds());
		if (distanceTraveled >= maxTravel - MAX_TRAVEL_EPSILON || hitWall)
			phase = Phase::Returning;
	} else {
		const sf::Vector2f toPlayer = playerPos - pos;
		const float distance = std::hypot(toPlayer.x, toPlayer.y);
		if (distance < CATCH_RADIUS)
			return true;
		velocity = (toPlayer / distance) * RETURN_SPEED;
		if (RETURN_SPEED * dt >= distance)
			return true;
		pos += velocity * dt;
	}

	sprite.setPosition(pos);
	return false;
}

void HatProjectile::draw(sf::RenderWindow &window) const
{
	window.draw(sprite);
}

sf::FloatRect HatProjectile::getBounds() const noexcept
{
	constexpr float halfSize = FRAME_SIZE / 2.f;
	return {{pos.x - halfSize, pos.y - halfSize}, {static_cast<float>(FRAME_SIZE), static_cast<float>(FRAME_SIZE)}};
}
