#pragma once

#include "../data/data_structs.hpp"
#include "../../ext/nlohmann/json.hpp"
#include "render_system.hpp"
#include <unordered_map>

class RenderSystem;

struct UpgradeTree {
	std::unordered_map<std::string, UpgradeInfo> upgrades;

	void add_upgrade(std::string name, UpgradeInfo upgrade) {
		upgrades.emplace(name, upgrade);
	}

	UpgradeInfo get_upgrade(std::string name) {
		return upgrades[name];
	}

	bool has_upgrade(std::string name) {
		return upgrades.find(name) != upgrades.end();
	}
};

class UpgradeSystem {

private:

	static inline std::string upgrade_tree_names[] = { "player_upgrades", "parry_upgrades" };
	std::array<UpgradeTree, std::size(upgrade_tree_names)> upgrade_trees;

	// Item icons
	std::unordered_map<std::string, GLuint> upgrade_handles;

	void load_upgrade_tree(RenderSystem& renderer, std::string name, UpgradeTree& tree);
	Modifier get_modifier_from_json(nlohmann::json item, std::string bonus_name);

public:

	void init(RenderSystem& renderer);

	void get_current_upgrades(std::vector<UpgradeInfo>& out);

	bool try_buy_upgrade(UpgradeInfo upgrade);

	UpgradeTree get_upgrade_tree(std::string tree_name);

	bool check_prerequisites(UpgradeInfo upgrade);

	std::string get_upgrade_name(int index);
};