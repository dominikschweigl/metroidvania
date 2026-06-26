#pragma once
#include "../entities/enemies/base_enemy.h"
#include "../entities/enemies/bosses/segfault_boss/segfault_boss.h"
#include "../entities/enemies/bosses/transistor_boss/transistor_boss.h"
#include "../entities/enemies/capacitor/capacitor.h"
#include "../entities/enemies/race_condition_slime/race_condition_slime.h"
#include "../entities/enemies/recursion_golem/recursion_golem.h"
#include "../entities/enemies/resistor_bug/resistor_bug.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <unordered_map>

using json = nlohmann::json;

struct EnemyFactory {
	static std::unique_ptr<BaseEnemy> create(const json &j)
	{
		if (j.empty())
			return nullptr;

		using Creator = std::function<std::unique_ptr<BaseEnemy>()>;
		static const std::unordered_map<std::string, Creator> table = {
		    {"RaceConditionSlime",
		     [] { return std::make_unique<RaceConditionSlime>(sf::Vector2f{}, BaseEnemy::DROP_CHANCE); }},
		    {"Capacitor", [] { return std::make_unique<Capacitor>(sf::Vector2f{}, BaseEnemy::DROP_CHANCE); }},
		    {"ResistorBug", [] { return std::make_unique<ResistorBug>(sf::Vector2f{}, BaseEnemy::DROP_CHANCE); }},
		    {"TransistorBoss", [] { return std::make_unique<TransistorBoss>(sf::Vector2f{}, BaseEnemy::DROP_CHANCE); }},
		    {"SegfaultBoss", [] { return std::make_unique<SegfaultBoss>(sf::Vector2f{}); }},
		    {"RecursionGolem",
		     [] {
			     return std::make_unique<RecursionGolem>(sf::Vector2f{}, RecursionGolem::DEFAULT_SIZE,
			                                             BaseEnemy::DROP_CHANCE);
		     }},
		};

		const std::string type = j.value("type", "");
		auto it = table.find(type);
		if (it == table.end()) {
			std::cerr << "EnemyFactory: unknown type '" << type << "'\n";
			return nullptr;
		}

		auto enemy = it->second();
		enemy->deserialize(j);
		return enemy;
	}
};
