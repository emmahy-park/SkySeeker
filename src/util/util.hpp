#pragma once

#include "../systems/upgrade_system.hpp"

class Util {
public:
	static vec2 get_position_from_coord(vec2 coord);
	static void get_bonus_string_from_upgrade(std::string& str, UpgradeInfo upgrade);
	static void get_bonus_string_from_items(std::string& str, ItemInfo iteminfo);
};

float hermite(float t, float p0, float p1, float m0, float m1);
