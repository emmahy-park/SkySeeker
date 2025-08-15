
#include <iostream>
#include "item_system.hpp"
#include "world_init.hpp"
#include "../data/items.hpp"
#include "../tinyECS/registry.hpp"

Modifier ItemSystem::get_modifier_from_json(nlohmann::json item, std::string bonus_name) {
	Modifier modifier;
	modifier.flat_bonus = item[bonus_name]["flat"];
	modifier.percent_bonus = static_cast<float>(item[bonus_name]["percent"]) / 100.f;

	return modifier;
}

void ItemSystem::load_items(RenderSystem& renderer) {
	std::ifstream f(json_path("items.json"));
	auto data = nlohmann::json::parse(f);

	for (int id = 0; id < data.size(); id++) {
		auto item = data[id];

		ItemInfo current_item;
		
		current_item.id = id;
		current_item.item_name = item["item_name"];
		current_item.description = item["description"];
		current_item.texture_name = item["texture_name"];

		current_item.rarity = ITEM_RARITY_STRINGS[item["rarity"]];
		current_item.spawn_rate = item["spawn_rate"];

		current_item.speed_bonus = get_modifier_from_json(item, "speed_bonus");
		current_item.health_bonus = get_modifier_from_json(item, "health_bonus");
		current_item.attack_bonus = get_modifier_from_json(item, "attack_bonus");
		current_item.defense_bonus = get_modifier_from_json(item, "defense_bonus");
		current_item.special_attack_bonus = get_modifier_from_json(item, "special_attack_bonus");

		current_item.crit_rate_bonus = item["crit_rate_bonus"];
		current_item.crit_damage_bonus = item["crit_damage_bonus"];
		current_item.healing_percent = item["healing_percent"];

		GLuint handle;
		std::string item_texture_path = textures_path(std::string ("items/" + current_item.texture_name));
        renderer.loadGlTextures(&handle, &item_texture_path, 1);
		item_handles[current_item.texture_name] = handle;

		items.push_back(current_item);

		if (item_rarities[current_item.rarity].find(current_item.item_name) != item_rarities[current_item.rarity].end()) {

			std::cerr << "Item with name " << current_item.item_name << "is already in the map" << std::endl;

			assert("Item is already in map");
		}
		
		item_rarities[current_item.rarity][current_item.item_name] = current_item; // TODO: complete

		if (current_item.rarity == SPECIAL) {

		}
	}

}


void ItemSystem::init(RenderSystem& renderer) {
	load_items(renderer);

	for (int i = 0; i < RARITY_COUNT; i++) {
		item_rarity_all_spawn_rates[i] = 0.f;
		auto& item_pool = item_rarities[i];
		for (auto iter = item_pool.begin(); iter != item_pool.end(); iter++) {
			item_rarity_all_spawn_rates[i] += iter->second.spawn_rate;
		}
	}
	
}

void ItemSystem::step(float elapsed_ms) {
	for (Entity& e : registry.items.entities) {
		Item& i = registry.items.get(e);

		if (i.is_pickup) {
			i.time_alive += elapsed_ms;

			Entity& player_entity = registry.players.entities[0];
			Position& player_pos = registry.positions.get(player_entity);

			Position& item_pos = registry.positions.get(e);

			vec2 difference = (player_pos.position - item_pos.position);

			float distance_squared = difference.x * difference.x + difference.y * difference.y;

			if (i.time_alive > ITEM_PICKUP_MIN_DELAY) {
				i.can_pickup = true;
			}

			if (i.time_alive > ITEM_PICKUP_SPREAD_TIME * 0.5f && distance_squared < ITEM_PICKUP_RANGE * ITEM_PICKUP_RANGE) {
				Velocity& item_vel = registry.velocities.get(e);
				item_vel.velocity += normalize(difference) * ITEM_PICKUP_ACCELERATION;
			}
			else if (i.time_alive > ITEM_PICKUP_SPREAD_TIME) {
				Velocity& item_vel = registry.velocities.get(e);
				item_vel.velocity = vec2(0, 0);
			}
		}
	}
}

ItemInfo ItemSystem::get_random_item() {

	float probability = static_cast<float>(rand()) / RAND_MAX;
	ItemRarity rarity = COMMON;

	for (int i = 0; i < ITEM_RARITY_PROBABILITIES.size(); i++) {
		float rarity_prob = ITEM_RARITY_PROBABILITIES[i];
		probability -= rarity_prob;
		if (probability <= 0) {
			rarity = static_cast<ItemRarity>(i);
			break;
		}
	}

	probability = static_cast<float>(rand()) / RAND_MAX;

	auto& item_pool = item_rarities[rarity];
	int index = 0;
	for (auto iter = item_pool.begin(); iter != item_pool.end(); iter++) {

		probability -= iter->second.spawn_rate / item_rarity_all_spawn_rates[rarity];
		if (probability <= 0) {
			return iter->second;
		}

		index++;
	}

	return items[0]; // This should never be hit unless there are empty item_pools
}

Entity ItemSystem::createPickupItem(PICKUP_ITEMS item_type, vec2 start_position, vec2 end_position, float value) {
	std::string item_name = PICKUP_ITEM_STRINGS[item_type];

	ItemInfo i = item_rarities[SPECIAL][item_name];

	Entity item_entity = createItem(i, true);

	Item& item = registry.items.get(item_entity);
	item.value = value;
	item.can_pickup = false;

	vec2 vel = end_position - start_position;

	Velocity& v = registry.velocities.emplace(item_entity);
	v.velocity = vel / ITEM_PICKUP_SPREAD_TIME * 1000.f;

	Collidable& c = registry.collidables.emplace(item_entity);

	Position& pos = registry.positions.emplace(item_entity);
	pos.position = start_position;
	pos.layer = 2;
	pos.scale = ITEM_SIZE;

	return item_entity;
}