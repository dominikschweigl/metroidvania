#pragma once
#include <functional>
#include <iostream>
#include <nlohmann/json.hpp>
#include <unordered_map>

using json = nlohmann::json;

// Segfault Boss
#include "../entities/enemies/bosses/segfault_boss/states/death_state.h"
#include "../entities/enemies/bosses/segfault_boss/states/fork_state.h"
#include "../entities/enemies/bosses/segfault_boss/states/null_spear_attack_state.h"
#include "../entities/enemies/bosses/segfault_boss/states/recover_state.h"
#include "../entities/enemies/bosses/segfault_boss/states/roaming_state.h"
#include "../entities/enemies/bosses/segfault_boss/states/stage2_transition_state.h"
#include "../entities/enemies/bosses/segfault_boss/states/stage3_transition_state.h"
#include "../entities/enemies/bosses/segfault_boss/states/summon_state.h"

// Transistor Boss
#include "../entities/enemies/bosses/transistor_boss/states/charge_attack_state.h"
#include "../entities/enemies/bosses/transistor_boss/states/charge_attack_windup_state.h"
#include "../entities/enemies/bosses/transistor_boss/states/death_state.h"
#include "../entities/enemies/bosses/transistor_boss/states/recover_state.h"
#include "../entities/enemies/bosses/transistor_boss/states/roaming_state.h"
#include "../entities/enemies/bosses/transistor_boss/states/shoot_attack_state.h"
#include "../entities/enemies/bosses/transistor_boss/states/stage2_recover_state.h"
#include "../entities/enemies/bosses/transistor_boss/states/summon_state.h"

// Capacitor
#include "../entities/enemies/capacitor/states/flee_state.h"
#include "../entities/enemies/capacitor/states/hover_state.h"
#include "../entities/enemies/capacitor/states/shoot_state.h"
#include "../entities/enemies/capacitor/states/swoop_state.h"

// Race Condition Slime
#include "../entities/enemies/race_condition_slime/states/attack_state.h"
#include "../entities/enemies/race_condition_slime/states/chase_state.h"
#include "../entities/enemies/race_condition_slime/states/idle_state.h"
#include "../entities/enemies/race_condition_slime/states/recover_state.h"
#include "../entities/enemies/race_condition_slime/states/windup_state.h"

// Resistor Bug
#include "../entities/enemies/resistor_bug/states/chase_state.h"
#include "../entities/enemies/resistor_bug/states/idle_state.h"
#include "../entities/enemies/resistor_bug/states/jump_attack_state.h"
#include "../entities/enemies/resistor_bug/states/recover_state.h"

// Recursion Golem
#include "../entities/enemies/recursion_golem/states/attack_state.h"
#include "../entities/enemies/recursion_golem/states/chase_state.h"
#include "../entities/enemies/recursion_golem/states/explode_state.h"
#include "../entities/enemies/recursion_golem/states/idle_state.h"
#include "../entities/enemies/recursion_golem/states/windup_state.h"

struct EnemyStateFactory {
	static std::unique_ptr<EnemyState> create(const json &j)
	{
		if (j.empty() || !j.contains("type"))
			return nullptr;

		using Creator = std::function<std::unique_ptr<EnemyState>()>;
		static const std::unordered_map<std::string, Creator> table = {
		    // Segfault Boss
		    {"SegfaultRoamingState", [] { return std::make_unique<segfault_boss::RoamingState>(); }},
		    {"NullSpearAttackState", [] { return std::make_unique<segfault_boss::NullSpearAttackState>(); }},
		    {"SegfaultRecoverState", [] { return std::make_unique<segfault_boss::RecoverState>(); }},
		    {"SegfaultDeathState", [] { return std::make_unique<segfault_boss::DeathState>(); }},
		    {"SegfaultStage2TransitionState", [] { return std::make_unique<segfault_boss::Stage2TransitionState>(); }},
		    {"SegfaultSummonState", [] { return std::make_unique<segfault_boss::SummonState>(); }},
		    {"SegfaultStage3TransitionState", [] { return std::make_unique<segfault_boss::Stage3TransitionState>(); }},
		    {"SegfaultForkState", [] { return std::make_unique<segfault_boss::ForkState>(); }},

		    // Transistor Boss
		    {"ChargeAttackState", [] { return std::make_unique<transistor_boss::ChargeAttackState>(); }},
		    {"ChargeAttackWindupState", [] { return std::make_unique<transistor_boss::ChargeAttackWindupState>(); }},
		    {"DeathState", [] { return std::make_unique<transistor_boss::DeathState>(); }},
		    {"RecoverState", [] { return std::make_unique<transistor_boss::RecoverState>(); }},
		    {"RoamingState", [] { return std::make_unique<transistor_boss::RoamingState>(); }},
		    {"ShootAttackState", [] { return std::make_unique<transistor_boss::ShootAttackState>(); }},
		    {"Stage2RecoverState", [] { return std::make_unique<transistor_boss::Stage2RecoverState>(); }},
		    {"SummonState", [] { return std::make_unique<transistor_boss::SummonState>(); }},

		    // Capacitor
		    {"FleeState", [] { return std::make_unique<capacitor::FleeState>(); }},
		    {"HoverState", [] { return std::make_unique<capacitor::HoverState>(); }},
		    {"ShootState", [] { return std::make_unique<capacitor::ShootState>(); }},
		    {"SwoopState", [] { return std::make_unique<capacitor::SwoopState>(); }},

		    // Race Condition Slime
		    {"AttackState", [] { return std::make_unique<rc_slime::AttackState>(); }},
		    {"ChaseState", [] { return std::make_unique<rc_slime::ChaseState>(); }},
		    {"IdleState", [] { return std::make_unique<rc_slime::IdleState>(); }},
		    {"RecoverState", [] { return std::make_unique<rc_slime::RecoverState>(); }},
		    {"WindUpState", [] { return std::make_unique<rc_slime::WindUpState>(); }},

		    // Resistor Bug
		    {"ResistorIdleState", [] { return std::make_unique<resistor_bug::IdleState>(); }},
		    {"ResistorChaseState", [] { return std::make_unique<resistor_bug::ChaseState>(); }},
		    {"ResistorJumpAttackState", [] { return std::make_unique<resistor_bug::JumpAttackState>(); }},
		    {"ResistorRecoverState", [] { return std::make_unique<resistor_bug::RecoverState>(); }},

		    // Recursion Golem
		    {"GolemIdleState", [] { return std::make_unique<recursion_golem::IdleState>(); }},
		    {"GolemChaseState", [] { return std::make_unique<recursion_golem::ChaseState>(); }},
		    {"GolemWindUpState", [] { return std::make_unique<recursion_golem::WindUpState>(); }},
		    {"GolemAttackState", [] { return std::make_unique<recursion_golem::AttackState>(); }},
		    {"GolemExplodeState", [] { return std::make_unique<recursion_golem::ExplodeState>(); }},
		};

		const std::string type = j.value("type", "");
		auto it = table.find(type);
		if (it == table.end()) {
			std::cerr << "EnemyStateFactory: unknown type '" << type << "'\n";
			return nullptr;
		}

		return it->second();
	}
};
