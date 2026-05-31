#include "world.h"
#include "../entities/enemies/race_condition_slime/race_condition_slime.h"
#include "../items/chewing_gum_item.h"
#include "../items/hat_item.h"
#include "../items/healing_potion_item.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <tileson.hpp>

void World::loadTilesets(tson::Map &map)
{
	AssetManager &am = AssetManager::getInstance();

	for (tson::Tileset &tileset : map.getTilesets()) {
		for (tson::Tile &tile : tileset.getTiles()) {
			const int gid = tile.getGid();

			// Skip if already loaded
			if (tileTextures.count(gid))
				continue;

			// Each tile in a collection tileset has its own image path
			fs::path imagePath = tile.getImage();
			if (imagePath.empty())
				continue;

			imagePath = fs::absolute("data/maps/tilesets/" + imagePath.string());

			// Load into AssetManager and cache by GID
			auto texture = std::make_shared<sf::Texture>();
			if (!texture->loadFromFile(imagePath.string())) {
				std::cerr << "Failed to load tile texture: " << imagePath << "\n";
				continue;
			}

			auto pair = tileTextures.emplace(gid, std::cref(*texture));
		}
	}
}

void World::loadRoom(std::vector<std::unique_ptr<BaseEnemy>> &enemies, std::vector<std::unique_ptr<WorldItem>> &items,
                     const std::string &roomId, const std::string &file)
{
	tson::Tileson t;
	auto map = t.parse(fs::path(file));

	if (!map || map->getStatus() != tson::ParseStatus::OK) {
		std::cerr << "Failed to parse " << file << ": " << (map ? map->getStatusMessage() : "null map") << "\n";
		return;
	}

	Room room;
	room.width = map->getSize().x;
	room.height = map->getSize().y;

	// --- Load textures ---
	loadTilesets(*map);

	// --- Parse objects ---
	if (tson::Layer *objLayer = map->getLayer("Object Layer 1")) {
		for (auto &obj : objLayer->getObjects()) {
			const tson::Vector2i p = obj.getPosition();
			const std::string &name = obj.getName();

			if (name == "Player") {
				room.playerSpawn = {float(p.x), float(p.y)};
			} else if (name == "Door") {
				Door door;
				door.bounds = sf::FloatRect({float(p.x), float(p.y)}, {float(obj.getSize().x), float(obj.getSize().y)});
				tson::Property *targetRoomProp = obj.getProp("targetRoomId");
				if (targetRoomProp) {
					door.targetRoomId = targetRoomProp->getValue<std::string>();
				}
				room.doors.push_back(door);
			} else if (name == "RaceConditionEnemy") {
				enemies.push_back(std::make_unique<RaceConditionSlime>(sf::Vector2f{float(p.x), float(p.y)}));
			} else if (name == "ChewingGumItem") {
				items.push_back(std::make_unique<WorldItem>(sf::Vector2f{float(p.x), float(p.y)},
				                                            std::make_unique<ChewingGumItem>()));
			} else if (name == "HatItem") {
				items.push_back(
				    std::make_unique<WorldItem>(sf::Vector2f{float(p.x), float(p.y)}, std::make_unique<HatItem>()));
			} else if (name == "HealingPotionItem") {
				items.push_back(std::make_unique<WorldItem>(sf::Vector2f{float(p.x), float(p.y)},
				                                            std::make_unique<HealingPotionItem>()));
			}
		}
	}

	room.map = std::move(map); // move last, after all parsing is done

	if (currentRoomId.empty())
		setCurrentRoom(roomId);

	rooms[roomId] = std::move(room);
}

void World::setCurrentRoom(const std::string &roomId)
{
	if (rooms.find(roomId) != rooms.end()) {
		currentRoomId = roomId;
	} else {
		std::cerr << "Room " << roomId << " not found" << std::endl;
	}
}

const Room *World::getCurrentRoom() const
{
	if (currentRoomId.empty() || rooms.find(currentRoomId) == rooms.end()) {
		return nullptr;
	}
	return &rooms.at(currentRoomId);
}

float World::getWorldHeight() const
{
	const Room *room = getCurrentRoom();
	if (room == nullptr) {
		return 0.f;
	}
	return room->height * TILE_SIZE;
}

tson::Tile *World::getTileAtCoordinate(const sf::Vector2f &worldPos, const std::string &layerName) const
{
	if (currentRoomId.empty())
		return nullptr;
	const Room &room = rooms.at(currentRoomId);

	const int x = static_cast<int>(worldPos.x / TILE_SIZE);
	const int y = static_cast<int>(worldPos.y / TILE_SIZE);

	// No real map (test grid) — return nullptr, callers must use isSolidAtRect
	if (!room.map)
		return nullptr;

	tson::Layer *layer = room.map->getLayer(layerName);
	if (!layer)
		return nullptr;
	return layer->getTileData(x, y);
}

bool World::isSolidAtRect(const sf::FloatRect &rect) const
{
	if (currentRoomId.empty())
		return false;
	const Room &room = rooms.at(currentRoomId);

	const int left = static_cast<int>(rect.position.x / TILE_SIZE);
	const int right = static_cast<int>((rect.position.x + rect.size.x - 0.001f) / TILE_SIZE);
	const int top = static_cast<int>(rect.position.y / TILE_SIZE);
	const int bottom = static_cast<int>((rect.position.y + rect.size.y - 0.001f) / TILE_SIZE);

	if (room.map) {
		tson::Layer *layer = room.map->getLayer("Solid");
		if (!layer)
			return false;

		for (int y = top; y <= bottom; ++y) {
			for (int x = left; x <= right; ++x) {
				tson::Tile *tile = layer->getTileData(x, y);
				if (tile)
					return true;
			}
		}
		return false;
	}

	for (int y = std::max(top, 0); y <= std::min(bottom, room.height - 1); ++y) {
		for (int x = std::max(left, 0); x <= std::min(right, room.width - 1); ++x) {
			if (room.solidGrid[y][x])
				return true;
		}
	}
	return false;
}

void World::loadFromGrid(const std::vector<std::vector<int>> &grid)
{
	if (grid.empty())
		return;

	Room room;
	room.width = grid[0].size();
	room.height = grid.size();
	room.solidGrid.resize(room.height, std::vector<bool>(room.width, false));

	for (int y = 0; y < room.height; ++y)
		for (int x = 0; x < room.width; ++x)
			room.solidGrid[y][x] = (grid[y][x] != 0);

	rooms["default"] = std::move(room);
	if (currentRoomId.empty())
		currentRoomId = "default";
}

void World::draw(sf::RenderWindow &window, const sf::View &view) const
{
	if (currentRoomId.empty())
		return;
	const Room &room = rooms.at(currentRoomId);

	const sf::Vector2f center = view.getCenter();
	const sf::Vector2f size = view.getSize();

	const int left = static_cast<int>((center.x - size.x * 0.5f) / TILE_SIZE);
	const int right = static_cast<int>((center.x + size.x * 0.5f) / TILE_SIZE);
	const int top = static_cast<int>((center.y - size.y * 0.5f) / TILE_SIZE);
	const int bottom = static_cast<int>((center.y + size.y * 0.5f) / TILE_SIZE);

	for (const std::string &layerName : {"Foreground", "Solid"}) {
		tson::Layer *layer = room.map->getLayer(layerName);
		if (!layer)
			continue;

		for (int y = top; y <= bottom; ++y) {
			for (int x = left; x <= right; ++x) {
				tson::Tile *tile = layer->getTileData(x, y);
				if (!tile) {
					continue;
				}

				auto it = tileTextures.find(tile->getGid());

				sf::RectangleShape shape({float(TILE_SIZE), float(TILE_SIZE)});
				shape.setPosition({float(x * TILE_SIZE), float(y * TILE_SIZE)});

				if (it != tileTextures.end())
					shape.setTexture(&it->second);
				else
					shape.setFillColor(sf::Color(255, 0, 255));

				window.draw(shape);
			}
		}
	}

	for (const Door &door : room.doors) {
		sf::RectangleShape shape(door.bounds.size);
		shape.setPosition(door.bounds.position);
		shape.setFillColor(sf::Color(0, 255, 255, 128));
		window.draw(shape);
	}
}
