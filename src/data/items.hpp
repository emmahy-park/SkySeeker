#pragma once

#include "data_structs.hpp"
#include <unordered_map>

extern std::unordered_map<std::string, GLuint> item_handles;

enum PICKUP_ITEMS {
	SOULS,
	HEALTH
};

static inline std::unordered_map<PICKUP_ITEMS, std::string> PICKUP_ITEM_STRINGS = {
	{SOULS, "souls"},
	{HEALTH, "health"}
};

static inline std::unordered_map<std::string, PICKUP_ITEMS> PICKUP_ITEM_ENUM_MAP = {
	{"souls", SOULS},
	{"health", HEALTH}
};