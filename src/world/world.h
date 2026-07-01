#pragma once

#include "../core/asset_manager.h"
#include "../entities/enemies/base_enemy.h"
#include "../items/world_item.h"
#include "../utils/ItemFactory.hpp"
#include "Room.hpp"
#include <SFML/Graphics.hpp>
#include <filesystem>
#include <future>
#include <nlohmann/json.hpp>
#include <optional>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;
using json = nlohmann::json;

class Player;

// ── Async loading types ───────────────────────────────────────────────────────

struct RoomManifest {
	std::string filePath;
	std::vector<std::string> adjacentRoomIds;
};

enum class RoomLoadState { Unloaded, Loading, Ready };

// Tracks load state and in-flight future.
// The Room itself lives in World::rooms_ once state == Ready.
struct RoomSlot {
	RoomManifest manifest;
	RoomLoadState state = RoomLoadState::Unloaded;
	std::future<Room> future;
};

// ── World ─────────────────────────────────────────────────────────────────────

class World {
  public:
	static constexpr float TILE_SIZE = 32.f;

	explicit World(std::string worldName);
	~World() = default;
	World(World &&) = default;
	World &operator=(World &&) = default;
	World(const World &) = delete;
	World &operator=(const World &) = delete;

	// ── Room registration and loading ─────────────────────────────────────────

	// Register a room path without parsing it. Call this at startup for every room.
	void registerRoom(const std::string &id, const std::string &filePath, std::vector<std::string> adjacent = {});

	// Kick off background parsing. No-op if already loading or ready.
	void requestLoad(const std::string &id);

	// Block until the room is ready. Starts loading if not already in flight.
	void requireLoad(const std::string &id);

	// Poll all in-flight futures; promote Loading → Ready.
	// Call once per frame from GameScene::update.
	void pollFutures();

	// Legacy: parse and insert a room synchronously (used for loadFromGrid in tests).
	void loadRoom(const std::string &roomId, const std::string &tmjFile);

	// ── Room access ───────────────────────────────────────────────────────────

	void setCurrentRoom(const std::string &roomId);
	[[nodiscard]] const std::string &getCurrentRoomId() const { return currentRoomId_; }
	[[nodiscard]] Room *getCurrentRoom();
	[[nodiscard]] const Room *getCurrentRoom() const;
	[[nodiscard]] float getWorldHeight();

	[[nodiscard]] std::vector<std::string> getAdjacentRoomIds(const std::string &id) const;
	[[nodiscard]] bool isRoomReady(const std::string &id) const;

	// ── Tile queries ──────────────────────────────────────────────────────────

	[[nodiscard]] tson::Tile *getTileAtCoordinate(const sf::Vector2f &worldPos, const std::string &layerName) const;
	[[nodiscard]] bool isSolidAtRect(const sf::FloatRect &rect) const;
	[[nodiscard]] bool isSolidTile(int tileX, int tileY) const;

	// ── Test support ──────────────────────────────────────────────────────────

	void loadFromGrid(const std::vector<std::vector<int>> &grid);

	// ── Persistence ───────────────────────────────────────────────────────────

	// Peek at the save file to find which room the player was in — cheap, no Room construction.
	[[nodiscard]] std::string readSavedRoomId() const;

	void saveWorldData(Player &player);

	// Call after requireLoad(readSavedRoomId()) so the target room is already ready.
	void loadWorldData(Player &player);

	// ── Render / update ───────────────────────────────────────────────────────

	void draw(sf::RenderWindow &window, const sf::View &view, sf::FloatRect playerBounds) const;
	void update(float deltaTime);

	// ── Convenience passthrough ───────────────────────────────────────────────

	Door *getTouchingDoor(const sf::FloatRect &entityBounds) { return getCurrentRoom()->getTouchingDoor(entityBounds); }

	bool isTouchingSavepoint(const sf::FloatRect &entityBounds)
	{
		return getCurrentRoom()->isTouchingSavepoint(entityBounds);
	}

  private:
	std::string worldName_;

	// Ready rooms — authoritative storage once a slot reaches RoomLoadState::Ready.
	std::unordered_map<std::string, Room> rooms_;

	// Async state for every registered room.
	std::unordered_map<std::string, RoomSlot> slots_;

	std::string currentRoomId_;
	std::unordered_map<int, std::shared_ptr<sf::Texture>> tileTextures_;

	// Save deltas for rooms that hadn't finished loading when loadWorldData ran.
	std::unordered_map<std::string, json> pendingRoomDeltas_;

	// ── Parsing helpers (pure — safe to call off the main thread) ─────────────
	Room parseRoom(const std::string &filePath);
	void loadTilesets(tson::Map &map); // writes tileTextures_ — main thread only
	void parseObjectLayer(Room &room, tson::Map &map);
	void parseSavePoints(Room &room, tson::Map &map);
	void parseImageLayers(Room &room, tson::Map &map);

	// Promote a completed future into rooms_ and apply any pending delta.
	void promoteSlot(const std::string &id, Room &&room);
};
