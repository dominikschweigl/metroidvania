#pragma once
#include "../entities/enemies/base_enemy.h"
#include "../entities/enemies/bosses/segfault_boss/segfault_boss.h"
#include "../entities/enemies/bosses/transistor_boss/transistor_boss.h"
#include "../entities/enemies/capacitor/capacitor.h"
#include "../entities/enemies/race_condition_slime/race_condition_slime.h"
#include "../entities/enemies/recursion_golem/recursion_golem.h"
#include "../entities/enemies/resistor_bug/resistor_bug.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct EnemyFactory {
	static std::unique_ptr<BaseEnemy> create(const json &j)
	{
		if (j.empty())
			return nullptr;
		std::string type = j["type"];

		if (type == "RaceConditionSlime") {
			auto enemy = std::make_unique<RaceConditionSlime>(sf::Vector2f{0.f, 0.f}, BaseEnemy::DROP_CHANCE);
			enemy->deserialize(j);
			return enemy;
		}

		if (type == "Capacitor") {
			auto enemy = std::make_unique<Capacitor>(sf::Vector2f{0.f, 0.f}, BaseEnemy::DROP_CHANCE);
			enemy->deserialize(j);
			return enemy;
		}

		if (type == "ResistorBug") {
			auto enemy = std::make_unique<ResistorBug>(sf::Vector2f{0.f, 0.f}, BaseEnemy::DROP_CHANCE);
			enemy->deserialize(j);
			return enemy;
		}

		if (type == "TransistorBoss") {
			auto enemy = std::make_unique<TransistorBoss>(sf::Vector2f{0.f, 0.f}, BaseEnemy::DROP_CHANCE);
			enemy->deserialize(j);
			return enemy;
		}

		if (type == "SegfaultBoss") {
			auto enemy = std::make_unique<SegfaultBoss>(sf::Vector2f{0.f, 0.f});
			enemy->deserialize(j);
			return enemy;
		}

		if (type == "RecursionGolem") {
			const int size = j.contains("size") ? j["size"].get<int>() : RecursionGolem::DEFAULT_SIZE;
			auto enemy = std::make_unique<RecursionGolem>(sf::Vector2f{0.f, 0.f}, size, BaseEnemy::DROP_CHANCE);
			enemy->deserialize(j);
			return enemy;
		}

		return nullptr;
	}
};
