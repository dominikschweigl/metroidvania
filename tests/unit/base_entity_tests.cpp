#include <catch2/catch_test_macros.hpp>

#include "combat/hitbox.h"
#include "entities/base_entity.h"
#include <SFML/Graphics.hpp>
#include <type_traits>

namespace {

// Minimal concrete BaseEntity so the abstract base can be instantiated.
class TestEntity : public BaseEntity {
  public:
	TestEntity(sf::Vector2f spawnPos, float w, float h, int maxHealth, Team team)
	    : BaseEntity(spawnPos, w, h, maxHealth, team)
	{
	}

	bool invulnerableForTest = false;
	bool isInvulnerable() const noexcept override { return invulnerableForTest; }
};

} // namespace

TEST_CASE("BaseEntity: ctor seeds position, defaults rest of state")
{
	TestEntity e({100.f, 200.f}, 28.f, 32.f, 5, Team::Enemy);
	REQUIRE(e.getPosition() == sf::Vector2f{100.f, 200.f});
	REQUIRE(e.getVelocity() == sf::Vector2f{0.f, 0.f});
	REQUIRE_FALSE(e.isOnGroundFlag());
	REQUIRE(e.health.current == 5);
	REQUIRE(e.health.max == 5);
	REQUIRE(e.isAlive());
}

TEST_CASE("BaseEntity: setPosition / getPosition round-trip")
{
	TestEntity e({0.f, 0.f}, 28.f, 32.f, 5, Team::Enemy);
	const sf::Vector2f target{321.5f, 654.25f};
	e.setPosition(target);
	REQUIRE(e.getPosition() == target);
}

TEST_CASE("BaseEntity: setVelocity / getVelocity round-trip")
{
	TestEntity e({0.f, 0.f}, 28.f, 32.f, 5, Team::Enemy);
	e.setVelocity({-50.f, 75.f});
	REQUIRE(e.getVelocity() == sf::Vector2f{-50.f, 75.f});
}

TEST_CASE("BaseEntity: setVelocityX / setVelocityY mutate only one axis")
{
	TestEntity e({0.f, 0.f}, 28.f, 32.f, 5, Team::Enemy);
	e.setVelocity({10.f, 20.f});
	e.setVelocityX(99.f);
	REQUIRE(e.getVelocity() == sf::Vector2f{99.f, 20.f});
	e.setVelocityY(-7.f);
	REQUIRE(e.getVelocity() == sf::Vector2f{99.f, -7.f});
}

TEST_CASE("BaseEntity: resetVelocity zeros both axes")
{
	TestEntity e({0.f, 0.f}, 28.f, 32.f, 5, Team::Enemy);
	e.setVelocity({-50.f, 75.f});
	e.resetVelocity();
	REQUIRE(e.getVelocity() == sf::Vector2f{0.f, 0.f});
}

TEST_CASE("BaseEntity: setDirection / getDirection round-trip")
{
	TestEntity e({0.f, 0.f}, 28.f, 32.f, 5, Team::Enemy);
	e.setDirection(Direction::Left);
	REQUIRE(e.getDirection() == Direction::Left);
	e.setDirection(Direction::Right);
	REQUIRE(e.getDirection() == Direction::Right);
}

TEST_CASE("BaseEntity: setOnGround flips the ground flag")
{
	TestEntity e({0.f, 0.f}, 28.f, 32.f, 5, Team::Enemy);
	REQUIRE_FALSE(e.isOnGroundFlag());
	e.setOnGround(true);
	REQUIRE(e.isOnGroundFlag());
	e.setOnGround(false);
	REQUIRE_FALSE(e.isOnGroundFlag());
}

TEST_CASE("BaseEntity::getBounds is a width x height rectangle anchored at the feet")
{
	TestEntity e({400.f, 200.f}, 28.f, 40.f, 5, Team::Enemy);
	const sf::FloatRect b = e.getBounds();
	REQUIRE(b.position.x == 400.f - 14.f); // pos.x - width/2
	REQUIRE(b.position.y == 200.f - 40.f); // pos.y - height
	REQUIRE(b.size.x == 28.f);
	REQUIRE(b.size.y == 40.f);
}

TEST_CASE("BaseEntity::takeDamage drops health and flips isAlive at zero (boundary)")
{
	TestEntity e({0.f, 0.f}, 28.f, 32.f, 3, Team::Enemy);
	e.takeDamage(1);
	REQUIRE(e.health.current == 2);
	REQUIRE(e.isAlive());
	e.takeDamage(10); // clamps
	REQUIRE(e.health.current == 0);
	REQUIRE_FALSE(e.isAlive());
}

TEST_CASE("BaseEntity::getHurtbox carries team, health pointer, and isInvulnerable() result")
{
	TestEntity e({100.f, 200.f}, 28.f, 32.f, 5, Team::Player);
	const Hurtbox a = e.getHurtbox();
	REQUIRE(a.team == Team::Player);
	REQUIRE(a.health == &e.health);
	REQUIRE_FALSE(a.invulnerable);
	REQUIRE(a.bounds.position == e.getBounds().position);
	REQUIRE(a.bounds.size == e.getBounds().size);

	e.invulnerableForTest = true;
	REQUIRE(e.getHurtbox().invulnerable);
}

TEST_CASE("BaseEntity::collectHurtboxes default appends a single hurtbox")
{
	TestEntity e({0.f, 0.f}, 28.f, 32.f, 5, Team::Enemy);
	std::vector<Hurtbox> hurts;
	e.collectHurtboxes(hurts);
	REQUIRE(hurts.size() == 1);
	REQUIRE(hurts.front().health == &e.health);
}

TEST_CASE("BaseEntity::collectHitboxes default is a no-op (failure path)")
{
	TestEntity e({0.f, 0.f}, 28.f, 32.f, 5, Team::Enemy);
	std::vector<Hitbox> hits;
	e.collectHitboxes(hits);
	REQUIRE(hits.empty());
}

TEST_CASE("BaseEntity::onHit pushes away from hit, starts hurt-flash and knockback timers")
{
	TestEntity e({100.f, 200.f}, 28.f, 32.f, 5, Team::Player);
	e.setOnGround(true);

	const Hitbox rightHit{sf::FloatRect({120.f, 180.f}, {16.f, 16.f}), 1, Team::Enemy, 99};
	e.onHit(rightHit);

	REQUIRE(e.getVelocity().x < 0.f);
	REQUIRE(e.getVelocity().y < 0.f);
	REQUIRE_FALSE(e.isOnGroundFlag());
	REQUIRE(e.isKnockedBack());
	REQUIRE(e.isHurtFlashing());

	TestEntity left({100.f, 200.f}, 28.f, 32.f, 5, Team::Player);
	const Hitbox leftHit{sf::FloatRect({60.f, 180.f}, {16.f, 16.f}), 1, Team::Enemy, 100};
	left.onHit(leftHit);
	REQUIRE(left.getVelocity().x > 0.f);
}

TEST_CASE("BaseEntity::tickHurtTimers decays both timers to zero")
{
	TestEntity e({0.f, 0.f}, 28.f, 32.f, 5, Team::Player);
	const Hitbox hit{sf::FloatRect({100.f, 0.f}, {10.f, 10.f}), 1, Team::Enemy, 1};
	e.onHit(hit);
	REQUIRE(e.isKnockedBack());
	REQUIRE(e.isHurtFlashing());

	e.tickHurtTimers(BaseEntity::KNOCKBACK_DURATION + 0.01f);
	REQUIRE_FALSE(e.isKnockedBack());
	REQUIRE_FALSE(e.isHurtFlashing());
}

TEST_CASE("BaseEntity::getHurtbox publishes the entity as owner")
{
	TestEntity e({0.f, 0.f}, 28.f, 32.f, 5, Team::Enemy);
	const Hurtbox hurt = e.getHurtbox();
	REQUIRE(hurt.owner == &e);
}

TEST_CASE("BaseEntity: non-copyable, non-movable (Rule of Five)")
{
	STATIC_REQUIRE_FALSE(std::is_copy_constructible_v<BaseEntity>);
	STATIC_REQUIRE_FALSE(std::is_copy_assignable_v<BaseEntity>);
	STATIC_REQUIRE_FALSE(std::is_move_constructible_v<BaseEntity>);
	STATIC_REQUIRE_FALSE(std::is_move_assignable_v<BaseEntity>);
}
