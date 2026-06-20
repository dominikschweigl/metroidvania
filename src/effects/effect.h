#pragma once
#include "../core/asset_manager.h"
#include <string_view>

struct Effect {
	[[nodiscard]] static Effect jumpBoost() noexcept;
	[[nodiscard]] static Effect speed() noexcept;
	[[nodiscard]] static Effect damage() noexcept;
	[[nodiscard]] static Effect resistance() noexcept;

	[[nodiscard]] static Effect slow() noexcept;

	[[nodiscard]] TextureAsset icon() const noexcept { return icon_; }
	[[nodiscard]] std::string_view name() const noexcept { return name_; }
	[[nodiscard]] std::string_view effectId() const noexcept { return effectId_; }
	[[nodiscard]] float speedMultiplier() const noexcept { return speedMult_; }
	[[nodiscard]] float jumpMultiplier() const noexcept { return jumpMult_; }
	[[nodiscard]] float damageMultiplier() const noexcept { return damageMult_; }
	[[nodiscard]] float damageResistance() const noexcept { return damageRes_; }

	float remainingDuration;
	float totalDuration;

  private:
	Effect(float totalDuration, TextureAsset icon, std::string_view name, std::string_view effectId, float speedMult,
	       float jumpMult, float damageMult, float damageRes) noexcept;

	TextureAsset icon_;
	std::string_view name_;
	std::string_view effectId_;
	float speedMult_;
	float jumpMult_;
	float damageMult_;
	float damageRes_;
};
