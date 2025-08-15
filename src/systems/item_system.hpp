#pragma once

#include "render_system.hpp"
#include "../data/items.hpp"
#include "../data/data_structs.hpp"
#include "../../ext/nlohmann/json.hpp"
#include <unordered_map>

class ItemSystem {
	
	std::vector<ItemInfo> items; // create views with common/uncommon/rare/legendary?
	std::array<std::unordered_map<std::string, ItemInfo>, ItemRarity::RARITY_COUNT> item_rarities;
	std::array<float, ItemRarity::RARITY_COUNT> item_rarity_all_spawn_rates;

	void load_items(RenderSystem& renderer);
	Modifier get_modifier_from_json(nlohmann::json item, std::string bonus_name);

public:
	void init(RenderSystem& renderer);

	ItemInfo get_random_item();

	void step(float elapsed_ms);

	Entity createPickupItem(PICKUP_ITEMS item_type, vec2 start_position, vec2 end_position, float value);
};


/**
To add a new item edit data/items.json, a template item is provided below:

Item template, can copy paste.
	{
		"item_name": "placeholder_sword",
		"description": "This is a legendary sword!",
		"texture_name": "placeholder_sword",
		"rarity": "COMMON",
		"spawn_rate": 1.0, // higher means more common
		"speed_bonus": {
			"flat": 0.0,
			"percent": 0.0
		},
		"health_bonus": {
			"flat": 0.0,
			"percent": 0.0
		},
		"attack_bonus": {
			"flat": 0.0,
			"percent": 0.0
		},
		"defense_bonus": {
			"flat": 0.0,
			"percent": 0.0
		},
		"special_attack_bonus": {
			"flat": 0.0,
			"percent": 0.0
		},
		"crit_rate_bonus": 0.0,
		"crit_damage_bonus": 0.0,
		"healing_percent": 0.0
	}
*/