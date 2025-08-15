#include "upgrade_system.hpp"
#include <iostream>
#include "../tinyECS/registry.hpp"

void UpgradeSystem::get_current_upgrades(std::vector<UpgradeInfo>& out) {
	assert(registry.gameStates.size() > 0);

	GameState& game_state = registry.gameStates.components[0];

	for (auto& upgrade : game_state.unlocked_upgrades) {
		for (int i = 0; i < upgrade_trees.size(); i++) {
			if (upgrade_tree_names[i] != "player_upgrades" && upgrade_tree_names[i] != "parry_upgrades") {// TODO: Update to be any generic special
				continue;
			}

			if (upgrade_trees[i].has_upgrade(upgrade)) {
				out.push_back(upgrade_trees[i].get_upgrade(upgrade));
				break;
			}
		}
	}


}

// TODO: extract, same as in item system
Modifier UpgradeSystem::get_modifier_from_json(nlohmann::json item, std::string bonus_name) {
	Modifier modifier;

	try {
		modifier.flat_bonus = item[bonus_name]["flat"];
		modifier.percent_bonus = static_cast<float>(item[bonus_name]["percent"]) / 100.f;
	}
	catch (...) {
		modifier = { 0, 0 };
	}

	return modifier;
}

void UpgradeSystem::load_upgrade_tree(RenderSystem& renderer, std::string name, UpgradeTree& tree) {
	std::ifstream f(json_path(name + ".json"));
	auto data = nlohmann::json::parse(f);

	for (int id = 0; id < data.size(); id++) {

		try {
			auto upgrade = data[id];

			UpgradeInfo current_upgrade;

			// Required
			current_upgrade.upgrade_name = upgrade["upgrade_name"];
			current_upgrade.description = upgrade["description"];
			current_upgrade.texture_name = upgrade["texture_name"];
			current_upgrade.cost = upgrade["cost"];

			try {
				current_upgrade.prerequisite_upgrades = upgrade["prerequisite_upgrades"];
			}
			catch (...) {
			}

			current_upgrade.speed_bonus = get_modifier_from_json(upgrade, "speed_bonus");
			current_upgrade.health_bonus = get_modifier_from_json(upgrade, "health_bonus");
			current_upgrade.attack_bonus = get_modifier_from_json(upgrade, "attack_bonus");
			current_upgrade.defense_bonus = get_modifier_from_json(upgrade, "defense_bonus");
			current_upgrade.special_attack_bonus = get_modifier_from_json(upgrade, "special_attack_bonus");

			try {
				current_upgrade.crit_rate_bonus = upgrade["crit_rate_bonus"];
			}
			catch (...) {}

			try {
				current_upgrade.crit_damage_bonus = upgrade["crit_damage_bonus"];
			}
			catch (...) {}

			current_upgrade.special_cooldown_reduction = get_modifier_from_json(upgrade, "special_cooldown_reduction");
			current_upgrade.special_duration = get_modifier_from_json(upgrade, "special_duration");
			
			GLuint handle;
			std::string upgrade_texture_path = textures_path(std::string("upgrades/" + current_upgrade.texture_name));
			renderer.loadGlTextures(&handle, &upgrade_texture_path, 1);
			upgrade_handles[current_upgrade.texture_name] = handle;

			tree.add_upgrade(current_upgrade.upgrade_name, current_upgrade);

		}
		catch (std::exception e) {
			std::cerr << "Error loading upgrade, error: " << e.what() << std::endl;
		}
	}

}

void UpgradeSystem::init(RenderSystem& renderer) {
	
	for (int i = 0; i < std::size(upgrade_tree_names); i++) {
		load_upgrade_tree(renderer, upgrade_tree_names[i], upgrade_trees[i]);
	}

}

bool UpgradeSystem::try_buy_upgrade(UpgradeInfo upgrade) {
	assert(registry.gameStates.size() > 0);

	GameState& game_state = registry.gameStates.components[0];
	
	if (game_state.souls < upgrade.cost) {
		return false;
	}

	if (!check_prerequisites(upgrade)) {
		return false;
	}

	game_state.souls -= upgrade.cost;
	game_state.unlocked_upgrades.push_back(upgrade.upgrade_name);

	return true;
}

UpgradeTree UpgradeSystem::get_upgrade_tree(std::string tree_name) {
	for (int i = 0; i < std::size(upgrade_tree_names); i++) {
		if (upgrade_tree_names[i] == tree_name) {
			return upgrade_trees[i];
		}
	}

	return upgrade_trees[0]; // TODO: Null?
}

std::string UpgradeSystem::get_upgrade_name(int index) {
	return upgrade_tree_names[index];
}

bool UpgradeSystem::check_prerequisites(UpgradeInfo upgrade) {
	assert(registry.gameStates.size() > 0);

	GameState& game_state = registry.gameStates.components[0];

	for (std::string pre_req : upgrade.prerequisite_upgrades) {
		if (std::find(game_state.unlocked_upgrades.begin(), game_state.unlocked_upgrades.end(), pre_req) == game_state.unlocked_upgrades.end()) {
			return false; // Does not have pre-reqs
		}
	}

	return true;
}