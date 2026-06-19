#pragma once
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

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
	static EnemyState *create(const json &j)
	{
		if (j.empty() || !j.contains("type"))
			return nullptr;

		const std::string type = j["type"];

		std::unique_ptr<EnemyState> state;

		// Segfault Boss States
		if (type == "SegfaultRoamingState")
			state = std::make_unique<segfault_boss::RoamingState>();

		if (type == "NullSpearAttackState")
			state = std::make_unique<segfault_boss::NullSpearAttackState>();

		if (type == "SegfaultRecoverState")
			state = std::make_unique<segfault_boss::RecoverState>();

		if (type == "SegfaultDeathState")
			state = std::make_unique<segfault_boss::DeathState>();

		if (type == "SegfaultStage2TransitionState")
			state = std::make_unique<segfault_boss::Stage2TransitionState>();

		if (type == "SegfaultSummonState")
			state = std::make_unique<segfault_boss::SummonState>();

		if (type == "SegfaultStage3TransitionState")
			state = std::make_unique<segfault_boss::Stage3TransitionState>();

		if (type == "SegfaultForkState")
			state = std::make_unique<segfault_boss::ForkState>();

		// Transistor Boss States
		if (type == "ChargeAttackState")
			state = std::make_unique<transistor_boss::ChargeAttackState>();

		if (type == "ChargeAttackWindupState")
			state = std::make_unique<transistor_boss::ChargeAttackWindupState>();

		if (type == "DeathState")
			state = std::make_unique<transistor_boss::DeathState>();

		if (type == "RecoverState")
			state = std::make_unique<transistor_boss::RecoverState>();

		if (type == "RoamingState")
			state = std::make_unique<transistor_boss::RoamingState>();

		if (type == "ShootAttackState")
			state = std::make_unique<transistor_boss::ShootAttackState>();

		if (type == "Stage2RecoverState")
			state = std::make_unique<transistor_boss::Stage2RecoverState>();

		if (type == "SummonState")
			state = std::make_unique<transistor_boss::SummonState>();

		// Capacitor States
		if (type == "FleeState")
			state = std::make_unique<capacitor::FleeState>();

		if (type == "HoverState")
			state = std::make_unique<capacitor::HoverState>();

		if (type == "ShootState")
			state = std::make_unique<capacitor::ShootState>();

		if (type == "SwoopState")
			state = std::make_unique<capacitor::SwoopState>();

		// Race Condition Slime
		if (type == "AttackState")
			state = std::make_unique<rc_slime::AttackState>();

		if (type == "ChaseState")
			state = std::make_unique<rc_slime::ChaseState>();

		if (type == "IdleState")
			state = std::make_unique<rc_slime::IdleState>();

		if (type == "RecoverState")
			state = std::make_unique<rc_slime::RecoverState>();

		if (type == "WindUpState")
			state = std::make_unique<rc_slime::WindUpState>();

		// Resistor Bug States
		if (type == "ResistorIdleState")
			state = std::make_unique<resistor_bug::IdleState>();

		if (type == "ResistorChaseState")
			state = std::make_unique<resistor_bug::ChaseState>();

		if (type == "ResistorJumpAttackState")
			state = std::make_unique<resistor_bug::JumpAttackState>();

		if (type == "ResistorRecoverState")
			state = std::make_unique<resistor_bug::RecoverState>();

		// Recursion Golem
		if (type == "GolemIdleState")
			state = std::make_unique<recursion_golem::IdleState>();

		if (type == "GolemChaseState")
			state = std::make_unique<recursion_golem::ChaseState>();

		if (type == "GolemWindUpState")
			state = std::make_unique<recursion_golem::WindUpState>();

		if (type == "GolemAttackState")
			state = std::make_unique<recursion_golem::AttackState>();

		if (type == "GolemExplodeState")
			state = std::make_unique<recursion_golem::ExplodeState>();

		return state.release();
	}
};
