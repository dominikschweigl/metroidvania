#include "world.h"
#include "../entities/enemies/bosses/segfault_boss/segfault_boss.h"
#include "../entities/enemies/bosses/transistor_boss/transistor_boss.h"
#include "../entities/enemies/capacitor/capacitor.h"
#include "../entities/enemies/race_condition_slime/race_condition_slime.h"
#include "../entities/enemies/recursion_golem/recursion_golem.h"
#include "../entities/enemies/resistor_bug/resistor_bug.h"
#include "../entities/player/player.h"
#include "../items/backup_disk_item.h"
#include "../items/chewing_gum_item.h"
#include "../items/damage_potion_item.h"
#include "../items/hat_item.h"
#include "../items/healing_potion_item.h"
#include "../items/jump_potion_item.h"
#include "../items/resistance_potion_item.h"
#include "../items/speed_potion_item.h"
#include "../items/usb_key_item.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <tileson.hpp>

World::World(const std::string worldName)
{
	this->worldName = worldName;
}

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

			tileTextures.emplace(gid, std::cref(*texture));
		}
	}
}

void World::loadRoom(const std::string &roomId, const std::string &file)
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
	room.needsToClearAllEnemies =
	    map->getProp("needsToClearAllEnemies") ? map->get<bool>("needsToClearAllEnemies") : false;

	// --- Load textures ---
	loadTilesets(*map);

	// --- Parse objects ---
	if (tson::Layer *objLayer = map->getLayer("Object Layer")) {
		for (auto &obj : objLayer->getObjects()) {
			const tson::Vector2i p = obj.getPosition();
			const std::string &name = obj.getName();

			if (name == "PlayerSpawn") {
				room.playerSpawns.push_back({float(p.x), float(p.y)});
				std::string direction = obj.get<std::string>("dir");
				if (direction == "left") {
					room.playerSpawnDirection = Direction::Left;
				} else if (direction == "right") {
					room.playerSpawnDirection = Direction::Right;
				}
			} else if (name == "Door") {
				Door door;
				door.bounds = sf::FloatRect({float(p.x), float(p.y)}, {float(obj.getSize().x), float(obj.getSize().y)});
				door.targetRoomId = obj.getProp("targetRoomId") ? obj.get<std::string>("targetRoomId") : "";
				door.targetSpawnIdx = obj.getProp("targetSpawnIdx") ? obj.get<int>("targetSpawnIdx") : 0;
				room.doors.push_back(door);
			} else if (name == "RaceConditionEnemy") {
				room.enemies_.push_back(std::make_unique<RaceConditionSlime>(sf::Vector2f{float(p.x), float(p.y)}));
			} else if (name == "TransistorBoss") {
				room.enemies_.push_back(std::make_unique<TransistorBoss>(sf::Vector2f{float(p.x), float(p.y)}));
			} else if (name == "SegfaultBoss") {
				room.enemies_.push_back(std::make_unique<SegfaultBoss>(sf::Vector2f{float(p.x), float(p.y)}));
			} else if (name == "Capacitor") {
				room.enemies_.push_back(std::make_unique<Capacitor>(sf::Vector2f{float(p.x), float(p.y)}));
			} else if (name == "ResistorBug") {
				room.enemies_.push_back(std::make_unique<ResistorBug>(sf::Vector2f{float(p.x), float(p.y)}));
			} else if (name == "RecursionGolem") {
				room.enemies_.push_back(std::make_unique<RecursionGolem>(sf::Vector2f{float(p.x), float(p.y)},
				                                                         RecursionGolem::DEFAULT_SIZE));
			} else if (name == "ChewingGumItem") {
				room.items_.push_back(std::make_unique<WorldItem>(sf::Vector2f{float(p.x), float(p.y)},
				                                                  std::make_unique<ChewingGumItem>()));
			} else if (name == "HatItem") {
				room.items_.push_back(
				    std::make_unique<WorldItem>(sf::Vector2f{float(p.x), float(p.y)}, std::make_unique<HatItem>()));
			} else if (name == "HealingPotionItem") {
				room.items_.push_back(std::make_unique<WorldItem>(sf::Vector2f{float(p.x), float(p.y)},
				                                                  std::make_unique<HealingPotionItem>()));
			} else if (name == "JumpPotionItem") {
				room.items_.push_back(std::make_unique<WorldItem>(sf::Vector2f{float(p.x), float(p.y)},
				                                                  std::make_unique<JumpPotionItem>()));
			} else if (name == "ResistancePotionItem") {
				room.items_.push_back(std::make_unique<WorldItem>(sf::Vector2f{float(p.x), float(p.y)},
				                                                  std::make_unique<ResistancePotionItem>()));
			} else if (name == "SpeedPotionItem") {
				room.items_.push_back(std::make_unique<WorldItem>(sf::Vector2f{float(p.x), float(p.y)},
				                                                  std::make_unique<SpeedPotionItem>()));
			} else if (name == "DamagePotionItem") {
				room.items_.push_back(std::make_unique<WorldItem>(sf::Vector2f{float(p.x), float(p.y)},
				                                                  std::make_unique<DamagePotionItem>()));
			} else if (name == "UsbKeyItem") {
				room.items_.push_back(
				    std::make_unique<WorldItem>(sf::Vector2f{float(p.x), float(p.y)}, std::make_unique<UsbKeyItem>()));
			} else if (name == "BackupDiskItem") {
				room.items_.push_back(std::make_unique<WorldItem>(sf::Vector2f{float(p.x), float(p.y)},
				                                                  std::make_unique<BackupDiskItem>()));
			}
		}
	}

	tson::Layer *layer = map->getLayer("Foreground");

	for (int y = 0; y < room.height; ++y) {
		for (int x = 0; x < room.width; ++x) {
			tson::Tile *tile = layer->getTileData(x, y);

			if (!tile)
				continue;

			// identify savepoint tile somehow
			const std::string tileType = tile->get<std::string>("type");
			if (tileType == "SavePoint") {
				auto &objectGroup = tile->getObjectgroup();

				for (auto &obj : objectGroup.getObjects()) {
					room.savePoints.push_back(SavePoint{
					    sf::FloatRect({x * TILE_SIZE + obj.getPosition().x, y * TILE_SIZE + obj.getPosition().y},
					                  {(float)obj.getSize().x, (float)obj.getSize().y})});
				}
			}
		}
	}

	for (auto &layer : map->getLayers()) {
		if (layer.getType() != tson::LayerType::ImageLayer)
			continue;

		auto texture = std::make_shared<sf::Texture>();

		auto path = fs::weakly_canonical(fs::absolute("maps/tilesets/" + layer.getImage()));

		if (texture->loadFromFile(path.string(), true)) {

			int width = 0;
			if (auto *prop = layer.getProp("imagewidth")) {
				auto w = prop->getValue<std::string>();
				std::cout << "w: " << w << std::endl;
				// width = static_cast<int>(w);
			}

			room.backgroundLayers.push_back({.texture = texture,
			                                 .position = {float(layer.getOffset().x), float(layer.getOffset().y)},
			                                 .parallax = {},
			                                 .repeatX = layer.hasRepeatX()});
		}
	}

	room.map = std::move(map); // move last, after all parsing is done
	rooms[roomId] = std::move(room);

	if (currentRoomId.empty())
		setCurrentRoom(roomId);
}

void World::setCurrentRoom(const std::string &roomId)
{
	if (rooms.find(roomId) != rooms.end()) {
		currentRoomId = roomId;
	} else {
		std::cerr << "Room " << roomId << " not found" << std::endl;
	}
}

Room *World::getCurrentRoom()
{
	if (currentRoomId.empty() || rooms.find(currentRoomId) == rooms.end()) {
		return nullptr;
	}
	return &rooms.at(currentRoomId);
}

float World::getWorldHeight()
{
	Room *room = getCurrentRoom();
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
	const int right = static_cast<int>((rect.position.x + rect.size.x) / TILE_SIZE);
	const int top = static_cast<int>(rect.position.y / TILE_SIZE);
	const int bottom = static_cast<int>((rect.position.y + rect.size.y) / TILE_SIZE);

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

void World::saveWorldData(Player &player)
{
	json j;
	try {
		j["player"] = player.serialize();

		j["currentRoom"] = getCurrentRoomId();

		j["rooms"] = json::array();
		for (const auto &room : rooms) {
			json j_room = room.second.serialize();
			j_room["id"] = room.first;
			j["rooms"].push_back(j_room);
		}

		std::ofstream file("saves/" + worldName + ".json");
		if (!file.is_open()) {
			std::cerr << "Failed to open save file\n";
			return;
		}

		file << j.dump(4);
	} catch (const std::exception &e) {
		std::cerr << "Serialization error: " << e.what() << "\n";
	}
}

void World::loadWorldData(Player &player)
{
	std::ifstream file("saves/" + worldName + ".json");
	if (!file.is_open()) {
		std::cerr << "Failed to open save file\n";
		return;
	}

	json j;
	try {
		file >> j;
	} catch (const std::exception &e) {
		std::cerr << "JSON parse error: " << e.what() << "\n";
		return;
	}

	try {
		if (j.contains("player")) {
			player.deserialize(j["player"]);
		}

		if (j.contains("rooms")) {
			for (const auto &j_room : j["rooms"]) {

				std::string id = "";
				if (j_room.contains("id"))
					id = j_room["id"];

				// If room doesn't exist yet, skip or create it
				if (rooms.find(id) == rooms.end()) {
					std::cerr << "Unknown room id in save: " << id << "\n";
					continue;
				}

				Room &room = rooms[id];

				room.deserialize(j_room);
			}
		}

		if (j.contains("currentRoom")) {
			setCurrentRoom(j["currentRoom"]);
		}

	} catch (const std::exception &e) {
		std::cerr << "World loading error: " << e.what() << "\n";
	}
}

void World::draw(sf::RenderWindow &window, const sf::View &view, const sf::FloatRect playerBounds) const
{
	if (currentRoomId.empty())
		return;
	const Room &room = rooms.at(currentRoomId);

	const sf::Vector2f center = view.getCenter();
	const sf::Vector2f size = view.getSize();

	const float roomWidth = World::TILE_SIZE * room.width;

	for (const ImageLayer &img : room.backgroundLayers) {
		if (img.repeatX) {
			for (float x = img.position.x; x < roomWidth; x += img.texture->getSize().x) {
				sf::Sprite sprite(*img.texture);
				sprite.setPosition({x, img.position.y});
				window.draw(sprite);
			}
		} else {
			sf::Sprite sprite(*img.texture);
			sprite.setPosition({img.position.x, img.position.y});
			window.draw(sprite);
		}
	}

	// Does not work correctly
	// sprite.setPosition(sf::Vector2f(img.position.x + center.x * (1.f - img.parallax.x),
	//                                 img.position.y + center.y * (1.f - img.parallax.y)));
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
		shape.setFillColor(sf::Color(119, 143, 129, 128));
		window.draw(shape);
	}

	const float playerX = playerBounds.position.x + playerBounds.size.x / 2.f;
	room.draw(window, playerBounds, playerX);
}

void World::update(float deltaTime, sf::FloatRect playerBounds)
{
	if (currentRoomId.empty())
		return;
	Room &room = rooms.at(currentRoomId);

	// for (auto &enemy : room.enemies_)
	// 	enemy->update(deltaTime, *this, playerBounds.position);
	// for (auto &item : room.items_)
	// 	item->update(deltaTime, *this);
	room.update(deltaTime, *this);
}
