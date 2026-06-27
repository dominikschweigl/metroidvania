#include "Room.hpp"

void Room::appendItem(std::unique_ptr<WorldItem> &newItem)
{
	items_.push_back(std::move(newItem));
}

bool Room::isTouchingSavepoint(const sf::FloatRect &entityBounds) const
{
	for (const SavePoint &savePoint : savePoints) {
		if (savePoint.bounds.findIntersection(entityBounds)) {
			return true;
		}
	}
	return false;
}

Door *Room::getTouchingDoor(const sf::FloatRect &entityBounds)
{
	for (Door &door : doors) {
		if (door.bounds.findIntersection(entityBounds)) {
			return &door;
		}
	}
	return nullptr;
}

void Room::update(float deltaTime, const World &world)
{
	enemies_.erase(
	    std::remove_if(enemies_.begin(), enemies_.end(), [](const auto &e) { return e->isReadyForRemoval(); }),
	    enemies_.end());
	items_.erase(std::remove_if(items_.begin(), items_.end(), [](const auto &i) { return i->isCollected(); }),
	             items_.end());
	updateInteractionIndicators(deltaTime);
}

void Room::draw(sf::RenderWindow &window, const sf::FloatRect playerBounds, const float playerX) const
{
	drawInteractionIndicators(window, playerBounds, playerX);
}

json Room::serialize() const
{
	json j;

	j["enemies"] = json::array();
	for (const auto &e : enemies_) {
		j["enemies"].push_back(e->serialize());
	}

	j["items"] = json::array();
	for (const auto &i : items_) {
		j["items"].push_back(i->serialize());
	}

	return j;
}

void Room::deserialize(const json &j)
{
	width = j.value("width", width);
	height = j.value("height", height);

	if (j.contains("enemies")) {
		enemies_.clear();
		for (const auto &e : j["enemies"]) {
			enemies_.push_back(EnemyFactory::create(e));
		}
	}

	if (j.contains("items")) {
		items_.clear();
		for (const auto &i : j["items"]) {
			items_.push_back(WorldItem::deserialize(i));
		}
	}
}

void Room::updateInteractionIndicators(const float deltaTime)
{
	for (Door &door : doors) {
		door.indicator.update(deltaTime);
	}
	for (SavePoint &savePoint : savePoints) {
		savePoint.indicator.update(deltaTime);
	}
}

void Room::drawInteractionIndicators(sf::RenderWindow &window, const sf::FloatRect playerBounds,
                                     const float playerX) const
{
	for (const Door &door : doors) {
		if (door.bounds.findIntersection(playerBounds)) {
			door.indicator.draw(window, door.bounds, playerX);
			return;
		}
	}
	for (const SavePoint &savePoint : savePoints) {
		if (savePoint.bounds.findIntersection(playerBounds)) {
			savePoint.indicator.draw(window, savePoint.bounds, playerX);
			return;
		}
	}
}
