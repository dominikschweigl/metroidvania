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

namespace {
template <typename T>
T getProp(tson::Object &obj, const std::string &key, T fallback = {})
{
	return obj.getProperties().hasProperty(key) ? obj.get<T>(key) : std::move(fallback);
}

std::vector<std::string> split(const std::string &str)
{
	std::vector<std::string> tokens;
	std::stringstream ss(str);
	std::string token;
	while (std::getline(ss, token, ',')) {
		if (!token.empty())
			tokens.push_back(token);
	}
	return tokens;
}

// Dispatch tables — eliminates the long if/else chains
using EnemyCreator = std::function<std::unique_ptr<BaseEnemy>(sf::Vector2f, float dropChance)>;

const std::unordered_map<std::string, EnemyCreator> &enemyCreators()
{
	static const std::unordered_map<std::string, EnemyCreator> table = {
	    {"RaceConditionEnemy", [](sf::Vector2f p, float d) { return std::make_unique<RaceConditionSlime>(p, d); }},
	    {"TransistorBoss", [](sf::Vector2f p, float d) { return std::make_unique<TransistorBoss>(p, d); }},
	    {"SegfaultBoss", [](sf::Vector2f p, float) { return std::make_unique<SegfaultBoss>(p); }},
	    {"Capacitor", [](sf::Vector2f p, float d) { return std::make_unique<Capacitor>(p, d); }},
	    {"ResistorBug", [](sf::Vector2f p, float d) { return std::make_unique<ResistorBug>(p, d); }},
	    {"RecursionGolem",
	     [](sf::Vector2f p, float d) { return std::make_unique<RecursionGolem>(p, RecursionGolem::DEFAULT_SIZE, d); }},
	};
	return table;
}

using ItemCreator = std::function<std::unique_ptr<WorldItem>(sf::Vector2f)>;

const std::unordered_map<std::string, ItemCreator> &itemCreators()
{
	static const std::unordered_map<std::string, ItemCreator> table = {
	    {"ChewingGumItem",
	     [](sf::Vector2f p) { return std::make_unique<WorldItem>(p, std::make_unique<ChewingGumItem>()); }},
	    {"HatItem", [](sf::Vector2f p) { return std::make_unique<WorldItem>(p, std::make_unique<HatItem>()); }},
	    {"HealingPotionItem",
	     [](sf::Vector2f p) { return std::make_unique<WorldItem>(p, std::make_unique<HealingPotionItem>()); }},
	    {"JumpPotionItem",
	     [](sf::Vector2f p) { return std::make_unique<WorldItem>(p, std::make_unique<JumpPotionItem>()); }},
	    {"ResistancePotionItem",
	     [](sf::Vector2f p) { return std::make_unique<WorldItem>(p, std::make_unique<ResistancePotionItem>()); }},
	    {"SpeedPotionItem",
	     [](sf::Vector2f p) { return std::make_unique<WorldItem>(p, std::make_unique<SpeedPotionItem>()); }},
	    {"DamagePotionItem",
	     [](sf::Vector2f p) { return std::make_unique<WorldItem>(p, std::make_unique<DamagePotionItem>()); }},
	    {"UsbKeyItem", [](sf::Vector2f p) { return std::make_unique<WorldItem>(p, std::make_unique<UsbKeyItem>()); }},
	    {"BackupDiskItem",
	     [](sf::Vector2f p) { return std::make_unique<WorldItem>(p, std::make_unique<BackupDiskItem>()); }},
	};
	return table;
}

} // namespace

World::World(const std::string worldName) : worldName(std::move(worldName)) {}

void World::loadTilesets(tson::Map &map)
{
	for (tson::Tileset &tileset : map.getTilesets()) {
		for (tson::Tile &tile : tileset.getTiles()) {
			const int gid = tile.getGid();
			if (tileTextures.count(gid))
				continue;

			fs::path imagePath = tile.getImage();
			if (imagePath.empty())
				continue;

			// Tiled paths are relative to the .tsj file
			imagePath = fs::absolute("data/maps/tilesets/" + imagePath.string());

			auto texture = std::make_shared<sf::Texture>();
			if (!texture->loadFromFile(imagePath.string())) {
				std::cerr << "Failed to load tile texture: " << imagePath << "\n";
				continue;
			}

			tileTextures.emplace(gid, std::move(texture));
		}
	}
}

void World::parseObjectLayer(Room &room, tson::Map &map)
{
	tson::Layer *objLayer = map.getLayer("Object Layer");
	if (!objLayer)
		return;

	for (auto &obj : objLayer->getObjects()) {
		const sf::Vector2f pos{float(obj.getPosition().x), float(obj.getPosition().y)};
		const std::string &name = obj.getName();

		if (name == "PlayerSpawn") {
			room.playerSpawns.push_back(pos);
			const std::string dir = getProp<std::string>(obj, "dir", "right");
			room.playerSpawnDirection = (dir == "left") ? Direction::Left : Direction::Right;
			continue;
		}

		if (name == "Door") {
			Door door;
			door.bounds = sf::FloatRect(pos, {float(obj.getSize().x), float(obj.getSize().y)});
			door.targetRoomId = getProp<std::string>(obj, "targetRoomId");
			door.targetSpawnIdx = getProp<int>(obj, "targetSpawnIdx", 0);
			door.needsToClearAllEnemies = getProp<bool>(obj, "needsToClearAllEnemies", false);
			door.locked = getProp<bool>(obj, "locked", false);
			room.doors.push_back(std::move(door));
			continue;
		}

		if (auto it = itemCreators().find(name); it != itemCreators().end()) {
			room.items_.push_back(it->second(pos));
			continue;
		}

		if (auto it = enemyCreators().find(name); it != enemyCreators().end()) {
			const float dropChance = getProp<float>(obj, "drop_chance", BaseEnemy::DROP_CHANCE);
			auto enemy = it->second(pos, dropChance);

			for (const auto &itemName : split(getProp<std::string>(obj, "drop_items"))) {
				json itemJson;
				itemJson["type"] = itemName;
				if (auto item = ItemFactory::create(itemJson))
					enemy->drop_items.push_back(std::move(item));
				else
					std::cerr << "Unknown drop item: '" << itemName << "'\n";
			}

			room.enemies_.push_back(std::move(enemy));
			continue;
		}
	}
}

void World::parseSavePoints(Room &room, tson::Map &map)
{
	tson::Layer *layer = map.getLayer("Foreground");
	if (!layer)
		return;

	for (int y = 0; y < room.height; ++y) {
		for (int x = 0; x < room.width; ++x) {
			tson::Tile *tile = layer->getTileData(x, y);
			if (!tile || tile->get<std::string>("type") != "SavePoint")
				continue;

			for (auto &obj : tile->getObjectgroup().getObjects()) {
				room.savePoints.push_back(SavePoint{sf::FloatRect(
				    {x * TILE_SIZE + float(obj.getPosition().x), y * TILE_SIZE + float(obj.getPosition().y)},
				    {float(obj.getSize().x), float(obj.getSize().y)})});
			}
		}
	}
}

void World::parseImageLayers(Room &room, tson::Map &map)
{
	for (auto &layer : map.getLayers()) {
		if (layer.getType() != tson::LayerType::ImageLayer)
			continue;

		auto texture = std::make_shared<sf::Texture>();
		// Tiled paths are relative to the .tmj file
		const fs::path path = fs::weakly_canonical(fs::absolute("data/maps/" + layer.getImage()));

		if (!texture->loadFromFile(path.string(), true)) {
			std::cerr << "Failed to load image layer: " << path << "\n";
			continue;
		}

		room.backgroundLayers.push_back({
		    .texture = std::move(texture),
		    .position = {float(layer.getOffset().x), float(layer.getOffset().y)},
		    .parallax = {},
		    .repeatX = layer.hasRepeatX(),
		});
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
	room.minimap_pixel_rect = {
	    {float(map->get<int>("minimap_pixel_rect_x")), float(map->get<int>("minimap_pixel_rect_y"))},
	    {float(map->get<int>("minimap_pixel_rect_w")), float(map->get<int>("minimap_pixel_rect_h"))}};
	room.world_index = static_cast<bool>(map->getProp("world_index"));

	loadTilesets(*map);
	parseObjectLayer(room, *map);
	parseSavePoints(room, *map);
	parseImageLayers(room, *map);

	room.map = std::move(map);
	rooms[roomId] = std::move(room);

	if (currentRoomId.empty())
		setCurrentRoom(roomId);
}

void World::setCurrentRoom(const std::string &roomId)
{
	if (rooms.count(roomId))
		currentRoomId = roomId;
	else
		std::cerr << "Room " << roomId << " not found\n";
}

Room *World::getCurrentRoom()
{
	if (currentRoomId.empty() || !rooms.count(currentRoomId))
		return nullptr;
	return &rooms.at(currentRoomId);
}

float World::getWorldHeight()
{
	Room *room = getCurrentRoom();
	return room ? room->height * TILE_SIZE : 0.f;
}

tson::Tile *World::getTileAtCoordinate(const sf::Vector2f &worldPos, const std::string &layerName) const
{
	if (currentRoomId.empty())
		return nullptr;
	const Room &room = rooms.at(currentRoomId);
	if (!room.map)
		return nullptr;

	const int x = static_cast<int>(worldPos.x / TILE_SIZE);
	const int y = static_cast<int>(worldPos.y / TILE_SIZE);

	tson::Layer *layer = room.map->getLayer(layerName);
	return layer ? layer->getTileData(x, y) : nullptr;
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

		for (int y = top; y <= bottom; ++y)
			for (int x = left; x <= right; ++x)
				if (layer->getTileData(x, y))
					return true;
		return false;
	}

	for (int y = std::max(top, 0); y <= std::min(bottom, room.height - 1); ++y)
		for (int x = std::max(left, 0); x <= std::min(right, room.width - 1); ++x)
			if (room.solidGrid[y][x])
				return true;
	return false;
}

void World::loadFromGrid(const std::vector<std::vector<int>> &grid)
{
	if (grid.empty())
		return;

	Room room;
	room.width = static_cast<int>(grid[0].size());
	room.height = static_cast<int>(grid.size());
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

		for (const auto &[id, room] : rooms) {
			json j_room = room.serialize();
			j_room["id"] = id;
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
		if (j.contains("player"))
			player.deserialize(j["player"]);

		if (j.contains("rooms")) {
			for (const auto &j_room : j["rooms"]) {
				const std::string id = j_room.value("id", "");
				if (!rooms.count(id)) {
					std::cerr << "Unknown room id in save: " << id << "\n";
					continue;
				}
				rooms[id].deserialize(j_room);
			}
		}

		if (j.contains("currentRoom"))
			setCurrentRoom(j["currentRoom"]);

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
	const float roomWidth = TILE_SIZE * room.width;

	for (const ImageLayer &img : room.backgroundLayers) {
		if (img.repeatX) {
			float x = img.position.x;
			while (x < roomWidth) {
				sf::Sprite sprite(*img.texture);
				sprite.setPosition({x, img.position.y});
				window.draw(sprite);
				x += img.texture->getSize().x
			}
		} else {
			sf::Sprite sprite(*img.texture);
			sprite.setPosition({img.position.x, img.position.y});
			window.draw(sprite);
		}
	}

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
				if (!tile)
					continue;

				sf::RectangleShape shape({float(TILE_SIZE), float(TILE_SIZE)});
				shape.setPosition({float(x * TILE_SIZE), float(y * TILE_SIZE)});

				auto it = tileTextures.find(tile->getGid());
				if (it != tileTextures.end())
					shape.setTexture(it->second.get()); // .get() because we now store shared_ptr
				else
					shape.setFillColor(sf::Color(255, 0, 255));

				window.draw(shape);
			}
		}
	}

	const float playerX = playerBounds.position.x + playerBounds.size.x / 2.f;
	room.draw(window, playerBounds, playerX);
}

void World::update(float deltaTime, sf::FloatRect playerBounds)
{
	if (currentRoomId.empty())
		return;
	rooms.at(currentRoomId).update(deltaTime, *this);
}
