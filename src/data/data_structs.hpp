#pragma once
#include "../common.hpp"
#include <array>

struct Modifier {
	float flat_bonus = 0;
	float percent_bonus = 0;

	void operator+= (Modifier const& other) {
		flat_bonus += other.flat_bonus;
		percent_bonus += other.percent_bonus;
	}
};

struct TextureCoords {
	vec2 top_left;
	vec2 bottom_right;
};

struct LinearAnimation {
	float animation_ms;
	float animation_step;
};

struct StepAnimation {
	float animation_ms;
	std::vector<float> *animation_steps;
};

struct SingleLinearAnimation {
	float animation_ms;
	float animation_step;
	void(*callback_fn)(Entity e);
};

union AnimationInfo {
	LinearAnimation la;
	StepAnimation sa;
	SingleLinearAnimation sla;
};

enum ItemRarity {
	COMMON,
	UNCOMMON,
	RARE,
	LEGENDARY,
	SPECIAL,
	RARITY_COUNT
}; // Something like this

static inline std::unordered_map<std::string, ItemRarity> ITEM_RARITY_STRINGS = {
	{"COMMON", COMMON},
	{"UNCOMMON", UNCOMMON},
	{"RARE", RARE},
	{"LEGENDARY", LEGENDARY},
	{"SPECIAL", SPECIAL}
};

const std::array<float, RARITY_COUNT> ITEM_RARITY_PROBABILITIES = {
	0.65f,
	0.35f,
	0.0f,
	0.0f,
	0.0f
};

/*
const std::array<float, RARITY_COUNT> ITEM_RARITY_PROBABILITIES = {
	0.45f,
	0.35f,
	0.15f,
	0.05f,
	0.0f
};
*/

struct ItemInfo {
	int id;
	int quantity;

	std::string item_name;
	std::string description;
	std::string texture_name;
	ItemRarity rarity; // Could be enum

	float spawn_rate;

	Modifier speed_bonus = { 0.0f, 0.0f };
	Modifier health_bonus = { 0, 0 };
	Modifier attack_bonus = { 0, 0 };
	Modifier defense_bonus = { 0, 0 };
	Modifier special_attack_bonus = { 0, 0 };
	float crit_rate_bonus = 0;
	float crit_damage_bonus = 0;
	float healing_percent = 0;
};

struct UpgradeInfo {

	std::string upgrade_name;
	std::string description;
	std::string texture_name;
	std::vector<std::string> prerequisite_upgrades;

	int cost;

	// Add any required modifiers
	Modifier speed_bonus = { 0.0f, 0.0f };
	Modifier health_bonus = { 0, 0 };
	Modifier attack_bonus = { 0, 0 };
	Modifier defense_bonus = { 0, 0 };
	Modifier special_attack_bonus = { 0, 0 };
	float crit_rate_bonus = 0;
	float crit_damage_bonus = 0;

	Modifier special_cooldown_reduction = { 0.0f, 0.0f };
	Modifier special_duration = { 0.0f, 0.0f };
};