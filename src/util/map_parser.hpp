
#pragma once

#include <string>
#include <vector>
#include "../ext/nlohmann/json.hpp"
#include "../systems/ai_system.hpp"

using json = nlohmann::json;

class MapLoader {
	// Map Texture Handle so we can release them afterwards
	std::vector<GLuint> map_texture_handles;
	int level = 0;
	std::string current_map = "";

public:
	void parseMaps(std::string path, RenderSystem* renderer, AISystem* ai);

	void unloadCurrentMap(RenderSystem* renderer, AISystem* ai);

	void loadNextMap(RenderSystem* renderer, AISystem* ai);

	int getLevel();

private:
	std::vector<std::string> getTilesetNames(json tilesets);

	std::vector<int> getTilesetFirstGID(json tilesets);

	std::vector<std::vector<int>> getTileProperties(std::vector<std::string> ts_file_names, std::vector<int> ts_first_gid);

	std::vector<vec2> getTilesetSize(std::vector<std::string> ts_file_names);

	std::vector<vec2> getTilesetDims(std::vector<std::string> ts_file_names);

};

Entity createMapTile(vec2 pos, float layer, vec2 bottom_left, vec2 top_right, bool collision, bool is_entrance, bool is_exit, int tileset_index, GLuint* handles);

Entity createEnemySpawn(vec2 pos);

Entity createItemSpawn(vec2 pos);

static inline MapLoader map_loader = MapLoader();