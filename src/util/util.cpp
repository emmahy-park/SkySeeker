#include "../common.hpp"
#include "util.hpp"

#include <sstream>

// Assume that the top left tile is (0, 0)
vec2 Util::get_position_from_coord(vec2 coord) {
	float x = coord.x * BASE_TILE_SIZE_WIDTH + BASE_TILE_SIZE_WIDTH / 2;
	float y = coord.y * BASE_TILE_SIZE_HEIGHT + BASE_TILE_SIZE_HEIGHT / 2; // Hopefully this gets compiled properly
	// float y = (coord.y + 0.5f) * BASE_TILE_SIZE_HEIGHT

	return vec2(x, y);
}

// [3] Hermite Curve function
// p(t) = (2t^3 - 3t^2 + 1) p_0 + (t^3 - 2t^2 + t)m_0 + (-2t^3 + 3t^2)p_1 + (t^3 - t^2)m_1
float hermite(float t, float p0, float p1, float m0, float m1) {
	float t3 = t * t * t;
	float t2 = t * t;
	return (2 * t3 - 3 * t2 + 1) * p0 + (t3 - 2 * t2 + t) * m0 + (-2 * t3 + 3 * t2) * p1 + (t3 - t2) * m1;
}

void Util::get_bonus_string_from_upgrade(std::string& string, UpgradeInfo upgrade) {
	std::stringstream bonuses;

	bonuses << std::setprecision(2);

	if (upgrade.attack_bonus.flat_bonus != 0)
		bonuses << "+" << upgrade.attack_bonus.flat_bonus << " attack" << std::endl;

	if (upgrade.attack_bonus.percent_bonus != 0)
		bonuses << "+" << upgrade.attack_bonus.percent_bonus * 100 << "% attack" << std::endl;



	if (upgrade.special_attack_bonus.flat_bonus != 0)
		bonuses << "+" << upgrade.special_attack_bonus.flat_bonus << " special attack" << std::endl;

	if (upgrade.special_attack_bonus.percent_bonus != 0)
		bonuses << "+" << upgrade.special_attack_bonus.percent_bonus * 100 << "% special attack" << std::endl;



	if (upgrade.health_bonus.flat_bonus != 0)
		bonuses << "+" << upgrade.health_bonus.flat_bonus << " health" << std::endl;

	if (upgrade.health_bonus.percent_bonus != 0)
		bonuses << "+" << upgrade.health_bonus.percent_bonus * 100 << "% health" << std::endl;



	if (upgrade.defense_bonus.flat_bonus != 0)
		bonuses << "+" << upgrade.defense_bonus.flat_bonus << " defense" << std::endl;

	if (upgrade.defense_bonus.percent_bonus != 0)
		bonuses << "+" << upgrade.defense_bonus.percent_bonus * 100 << "% defense" << std::endl;



	if (upgrade.speed_bonus.flat_bonus != 0)
		bonuses << "+" << upgrade.speed_bonus.flat_bonus << " speed" << std::endl;

	if (upgrade.speed_bonus.percent_bonus != 0)
		bonuses << "+" << upgrade.speed_bonus.percent_bonus * 100 << "% speed" << std::endl;


	if (upgrade.special_duration.flat_bonus != 0)
		bonuses << "+" << upgrade.special_duration.flat_bonus << " special duration" << std::endl;

	if (upgrade.special_duration.percent_bonus != 0)
		bonuses << "+" << upgrade.special_duration.percent_bonus * 100 << "% special duration" << std::endl;


	if (upgrade.special_cooldown_reduction.flat_bonus != 0)
		bonuses << "-" << upgrade.special_cooldown_reduction.flat_bonus << " special cooldown" << std::endl;

	if (upgrade.special_cooldown_reduction.percent_bonus != 0)
		bonuses << "-" << upgrade.special_cooldown_reduction.percent_bonus * 100 << "% special cooldown" << std::endl;

	string = bonuses.str();
}

// For New Feature: Item Stat Block
void Util::get_bonus_string_from_items(std::string& string, ItemInfo item) {
	std::stringstream bonuses;

	bonuses << std::setprecision(2);

	if (item.attack_bonus.flat_bonus > 0) bonuses << "+" << item.attack_bonus.flat_bonus << " attack\n" << std::endl;
	else if (item.attack_bonus.flat_bonus < 0) bonuses << item.attack_bonus.flat_bonus << " attack\n" << std::endl;

	if (item.attack_bonus.percent_bonus > 0) bonuses << "+" << item.attack_bonus.percent_bonus * 100 << "% attack\n" << std::endl;
	else if (item.attack_bonus.percent_bonus < 0) bonuses << item.attack_bonus.percent_bonus * 100 << "% attack\n" << std::endl;

	if (item.special_attack_bonus.flat_bonus > 0) bonuses << "+" << item.special_attack_bonus.flat_bonus << " special attack\n" << std::endl;
	else if (item.special_attack_bonus.flat_bonus < 0) bonuses << item.special_attack_bonus.flat_bonus << " special attack\n" << std::endl;

	if (item.special_attack_bonus.percent_bonus > 0) bonuses << "+" << item.special_attack_bonus.percent_bonus * 100 << "% special attack\n" << std::endl;
	else if (item.special_attack_bonus.percent_bonus < 0) bonuses << item.special_attack_bonus.percent_bonus * 100 << "% special attack\n" << std::endl;

	if (item.health_bonus.flat_bonus > 0) bonuses << "+" << item.health_bonus.flat_bonus << " health\n" << std::endl;
	else if (item.health_bonus.flat_bonus < 0) bonuses << item.health_bonus.flat_bonus << " health\n" << std::endl;

	if (item.health_bonus.percent_bonus > 0) bonuses << "+" << item.health_bonus.percent_bonus * 100 << "% health\n" << std::endl;
	else if (item.health_bonus.percent_bonus < 0) bonuses << item.health_bonus.percent_bonus * 100 << "% health\n" << std::endl;

	if (item.defense_bonus.flat_bonus > 0) bonuses << "+" << item.defense_bonus.flat_bonus << " defense\n" << std::endl;
	else if (item.defense_bonus.flat_bonus < 0) bonuses << item.defense_bonus.flat_bonus << " defense\n" << std::endl;

	if (item.defense_bonus.percent_bonus > 0) bonuses << "+" << item.defense_bonus.percent_bonus * 100 << "% defense\n" << std::endl;
	else if (item.defense_bonus.percent_bonus < 0) bonuses << item.defense_bonus.percent_bonus * 100 << "% defense\n" << std::endl;

	if (item.speed_bonus.flat_bonus > 0) bonuses << "+" << item.speed_bonus.flat_bonus << " speed\n" << std::endl;
	else if (item.speed_bonus.flat_bonus < 0) bonuses << item.speed_bonus.flat_bonus << " speed\n" << std::endl;

	if (item.speed_bonus.percent_bonus > 0) bonuses << "+" << item.speed_bonus.percent_bonus * 100 << "% speed\n" << std::endl;
	else if (item.speed_bonus.percent_bonus < 0) bonuses << item.speed_bonus.percent_bonus * 100 << "% speed\n" << std::endl;

	if (item.crit_rate_bonus > 0) bonuses << "+" << item.crit_rate_bonus << "% critical rate\n" << std::endl;
	else if (item.crit_rate_bonus < 0) bonuses << item.crit_rate_bonus << "% critical rate\n" << std::endl;

	if (item.crit_damage_bonus > 0) bonuses << "+" << item.crit_damage_bonus << "% critical damage\n" << std::endl;
	else if (item.crit_damage_bonus < 0) bonuses << item.crit_damage_bonus << "% critical damage\n" << std::endl;

	if (item.healing_percent > 0) bonuses << "+" << item.healing_percent << "% healing\n" << std::endl;
	else if (item.healing_percent < 0) bonuses << item.healing_percent << "% healing\n" << std::endl;

	string = bonuses.str();
}