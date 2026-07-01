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
#include <ctime>
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
	while (std::getline(ss, token, ','))
		if (!token.empty())
			tokens.push_back(token);
	return tokens;
}

using EnemyCreator = std::function<std::unique_ptr<BaseEnemy>(sf::Vector2f, float)>;

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

World::World(std::string worldName) : worldName_(std::move(worldName)) {}

void World::registerRoom(const std::string &id, const std::string &filePath, std::vector<std::string> adjacent)
{
	RoomSlot slot;
	slot.manifest.filePath = filePath;
	slot.manifest.adjacentRoomIds = std::move(adjacent);
	slot.state = RoomLoadState::Unloaded;
	slots_.emplace(id, std::move(slot));
}

void World::requestLoad(const std::string &id)
{
	auto it = slots_.find(id);
	if (it == slots_.end()) {
		std::cerr << "requestLoad: unknown room id '" << id << "'\n";
		return;
	}
	RoomSlot &slot = it->second;
	if (slot.state != RoomLoadState::Unloaded)
		return;
	slot.state = RoomLoadState::Loading;
	slot.future =
	    std::async(std::launch::async, [this, filePath = slot.manifest.filePath]() { return parseRoom(filePath); });
}

void World::requireLoad(const std::string &id)
{
	auto it = slots_.find(id);
	if (it == slots_.end()) {
		std::cerr << "requireLoad: unknown room id '" << id << "'\n";
		return;
	}
	RoomSlot &slot = it->second;

	if (slot.state == RoomLoadState::Ready)
		return;

	if (slot.state == RoomLoadState::Unloaded)
		requestLoad(id);

	promoteSlot(id, slot.future.get());
}

void World::pollFutures()
{
	for (auto &[id, slot] : slots_) {
		if (slot.state != RoomLoadState::Loading)
			continue;
		if (slot.future.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
			continue;
		promoteSlot(id, slot.future.get());
	}
}

void World::promoteSlot(const std::string &id, Room &&room)
{
	if (room.map)
		loadTilesets(*room.map);
	rooms_[id] = std::move(room);
	slots_.at(id).state = RoomLoadState::Ready;

	auto deltaIt = pendingRoomDeltas_.find(id);
	if (deltaIt != pendingRoomDeltas_.end()) {
		rooms_.at(id).deserialize(deltaIt->second);
		pendingRoomDeltas_.erase(deltaIt);
	}

	if (currentRoomId_.empty())
		currentRoomId_ = id;
}

void World::loadRoom(const std::string &roomId, const std::string &tmjFile)
{
	if (!slots_.count(roomId))
		registerRoom(roomId, tmjFile);

	Room room = parseRoom(tmjFile);
	promoteSlot(roomId, std::move(room));
}

Room World::parseRoom(const std::string &filePath)
{
	tson::Tileson t;
	auto map = t.parse(fs::path(filePath));

	if (!map || map->getStatus() != tson::ParseStatus::OK) {
		std::cerr << "Failed to parse " << filePath << ": " << (map ? map->getStatusMessage() : "null map") << "\n";
		return {};
	}

	Room room;
	room.width = map->getSize().x;
	room.height = map->getSize().y;
	room.minimap_pixel_rect = {
	    {float(map->get<int>("minimap_pixel_rect_x")), float(map->get<int>("minimap_pixel_rect_y"))},
	    {float(map->get<int>("minimap_pixel_rect_w")), float(map->get<int>("minimap_pixel_rect_h"))}};
	room.world_index = static_cast<bool>(map->getProp("world_index"));

	parseObjectLayer(room, *map);
	parseSavePoints(room, *map);
	parseImageLayers(room, *map);

	room.map = std::move(map);
	return room;
}

void World::loadTilesets(tson::Map &map)
{
	for (tson::Tileset &tileset : map.getTilesets()) {
		for (tson::Tile &tile : tileset.getTiles()) {
			const int gid = tile.getGid();
			if (tileTextures_.count(gid))
				continue;

			fs::path imagePath = tile.getImage();
			if (imagePath.empty())
				continue;

			imagePath = fs::absolute("data/maps/tilesets/" + imagePath.string());

			auto texture = std::make_shared<sf::Texture>();
			if (!texture->loadFromFile(imagePath.string())) {
				std::cerr << "Failed to load tile texture: " << imagePath << "\n";
				continue;
			}
			tileTextures_.emplace(gid, std::move(texture));
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

void World::setCurrentRoom(const std::string &roomId)
{
	if (rooms_.count(roomId))
		currentRoomId_ = roomId;
	else
		std::cerr << "setCurrentRoom: room '" << roomId << "' not ready\n";
}

Room *World::getCurrentRoom()
{
	if (currentRoomId_.empty() || !rooms_.count(currentRoomId_))
		return nullptr;
	return &rooms_.at(currentRoomId_);
}

const Room *World::getCurrentRoom() const
{
	if (currentRoomId_.empty() || !rooms_.count(currentRoomId_))
		return nullptr;
	return &rooms_.at(currentRoomId_);
}

float World::getWorldHeight()
{
	Room *room = getCurrentRoom();
	return room ? room->height * TILE_SIZE : 0.f;
}

std::vector<std::string> World::getAdjacentRoomIds(const std::string &id) const
{
	auto it = slots_.find(id);
	if (it == slots_.end())
		return {};
	return it->second.manifest.adjacentRoomIds;
}

bool World::isRoomReady(const std::string &id) const
{
	auto it = slots_.find(id);
	return it != slots_.end() && it->second.state == RoomLoadState::Ready;
}

tson::Tile *World::getTileAtCoordinate(const sf::Vector2f &worldPos, const std::string &layerName) const
{
	const Room *room = getCurrentRoom();
	if (!room || !room->map)
		return nullptr;

	const int x = static_cast<int>(worldPos.x / TILE_SIZE);
	const int y = static_cast<int>(worldPos.y / TILE_SIZE);

	tson::Layer *layer = room->map->getLayer(layerName);
	return layer ? layer->getTileData(x, y) : nullptr;
}

bool World::isSolidAtRect(const sf::FloatRect &rect) const
{
	const Room *room = getCurrentRoom();
	if (!room)
		return false;

	const int left = static_cast<int>(std::floor(rect.position.x / TILE_SIZE));
	const int right = static_cast<int>(std::floor((rect.position.x + rect.size.x) / TILE_SIZE));
	const int top = static_cast<int>(std::floor(rect.position.y / TILE_SIZE));
	const int bottom = static_cast<int>(std::floor((rect.position.y + rect.size.y) / TILE_SIZE));

	for (int y = top; y <= bottom; ++y)
		for (int x = left; x <= right; ++x)
			if (isSolidTile(x, y))
				return true;
	return false;
}

bool World::isSolidTile(int tileX, int tileY) const
{
	const Room *room = getCurrentRoom();
	if (!room)
		return false;
	if (tileX < 0 || tileY < 0 || tileX >= room->width || tileY >= room->height)
		return false;

	if (room->map) {
		tson::Layer *layer = room->map->getLayer("Solid");
		return layer && layer->getTileData(tileX, tileY) != nullptr;
	}
	return room->solidGrid[tileY][tileX];
}

void World::loadFromGrid(const std::vector<std::vector<int>> &grid)
{
	if (grid.empty())
		return;

	Room room;
	room.width = static_cast<int>(grid[0].size());
	room.height = static_cast<int>(grid.size());
	room.solidGrid.assign(room.height, std::vector<bool>(room.width, false));

	for (int y = 0; y < room.height; ++y)
		for (int x = 0; x < room.width; ++x)
			room.solidGrid[y][x] = (grid[y][x] != 0);

	registerRoom("default", "");
	promoteSlot("default", std::move(room));
}

std::string World::readSavedRoomId() const
{
	std::ifstream file("saves/" + worldName_ + ".json");
	if (!file.is_open())
		return "start_room";
	json j;
	try {
		file >> j;
	} catch (...) {
		return "start_room";
	}
	return j.value("currentRoom", "start_room");
}

void World::saveWorldData(Player &player)
{
	json j;
	try {
		j["player"] = player.serialize();
		j["currentRoom"] = currentRoomId_;
		j["rooms"] = json::array();

		for (const auto &[id, room] : rooms_) {
			json j_room = room.serialize();
			j_room["id"] = id;
			j["rooms"].push_back(j_room);
		}

		std::ofstream file("saves/" + worldName_ + ".json");
		if (!file.is_open()) {
			std::cerr << "Failed to open save file for writing\n";
			return;
		}
		file << j.dump(4);
	} catch (const std::exception &e) {
		std::cerr << "Serialization error: " << e.what() << "\n";
	}
}

void World::loadWorldData(Player &player)
{
	std::ifstream file("saves/" + worldName_ + ".json");
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
				if (!slots_.count(id)) {
					std::cerr << "Unknown room id in save: " << id << "\n";
					continue;
				}
				if (rooms_.count(id))
					rooms_.at(id).deserialize(j_room);
				else
					pendingRoomDeltas_[id] = j_room;
			}
		}

		if (j.contains("currentRoom"))
			setCurrentRoom(j["currentRoom"]);

	} catch (const std::exception &e) {
		std::cerr << "World loading error: " << e.what() << "\n";
	}
}

// ── Render / update ───────────────────────────────────────────────────────────

void World::draw(sf::RenderWindow &window, const sf::View &view, const sf::FloatRect playerBounds) const
{
	const Room *room = getCurrentRoom();
	if (!room)
		return;

	const sf::Vector2f center = view.getCenter();
	const sf::Vector2f size = view.getSize();
	const float roomWidth = TILE_SIZE * room->width;

	for (const ImageLayer &img : room->backgroundLayers) {
		if (img.repeatX) {
			float x = img.position.x;
			while (x < roomWidth) {
				sf::Sprite sprite(*img.texture);
				sprite.setPosition({x, img.position.y});
				window.draw(sprite);
				x += img.texture->getSize().x;
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
		tson::Layer *layer = room->map->getLayer(layerName);
		if (!layer)
			continue;

		for (int y = top; y <= bottom; ++y) {
			for (int x = left; x <= right; ++x) {
				tson::Tile *tile = layer->getTileData(x, y);
				if (!tile)
					continue;

				sf::RectangleShape shape({float(TILE_SIZE), float(TILE_SIZE)});
				shape.setPosition({float(x * TILE_SIZE), float(y * TILE_SIZE)});

				auto it = tileTextures_.find(tile->getGid());
				if (it != tileTextures_.end())
					shape.setTexture(it->second.get());
				else
					shape.setFillColor(sf::Color(255, 0, 255));

				window.draw(shape);
			}
		}
	}

	const float playerX = playerBounds.position.x + playerBounds.size.x / 2.f;
	room->draw(window, playerBounds, playerX);
}

void World::update(float deltaTime)
{
	Room *room = getCurrentRoom();
	if (room)
		room->update(deltaTime);
}
