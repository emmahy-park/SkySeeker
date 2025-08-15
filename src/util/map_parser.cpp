
#include "../../ext/nlohmann/json.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include "../common.hpp"
#include "../tinyECS/registry.hpp"
#include <vector>
#include "../systems/render_system.hpp"
#include "../systems/ai_system.hpp"
#include "../systems/world_init.hpp"

#include "map_parser.hpp"
using json = nlohmann::json;

void MapLoader::parseMaps(std::string path, RenderSystem* renderer, AISystem* ai) {

	std::cout << level << "\n";
	if (textures_path("map/city_0.json") == path) {
		level = 0;
	}

	std::ifstream f(path);
	json data = json::parse(f);
	json layers = data["layers"];
	int tileheight = data["tileheight"];
	int tilewidth = data["tilewidth"];
	int width = data["width"];
	json tilesets = data["tilesets"];

	current_map = path;

	// get all tileids (based on their gid in the map) that have collision
	std::vector<std::string> ts_file_names = getTilesetNames(tilesets);
	std::vector<int> ts_first_gid = getTilesetFirstGID(tilesets);
	std::vector<std::vector<int>> tileProperties = getTileProperties(ts_file_names, ts_first_gid);
	std::vector<int> collidables = tileProperties[0];
	std::vector<int> spawns = tileProperties[1];
	std::vector<int> entrance = tileProperties[2];
	std::vector<int> exit = tileProperties[3];
	std::vector<int> item = tileProperties[4];
	std::vector<int> light = tileProperties[5];
	// This is the size based on pixels
	std::vector<vec2> ts_sizes = getTilesetSize(ts_file_names);
	// This is the size based on number of tiles
	std::vector<vec2> ts_dimensions = getTilesetDims(ts_file_names);
	std::vector<std::string> path_names;
	
	for (int i = 0; i < ts_file_names.size(); i++) {
		path_names.push_back(textures_path("map/" + ts_file_names[i] + ".png"));
	}

	map_texture_handles = std::vector<GLuint>(ts_file_names.size(), 0);
	renderer->loadGlTextures(map_texture_handles.data(), path_names.data(), ts_file_names.size());

	// Loop through all layers
	for (int layer_num = 0; layer_num < layers.size(); layer_num++) {

		if (layers[layer_num].contains("objects")) {
			//auto object_layer = layers[layer_num]["objects"];
			//std::unordered_map<int, int> graphs; // graph, node
			//for (int obj = 0; obj < object_layer.size(); obj++) {
			//	vec2 pos = { object_layer[obj]["x"], object_layer[obj]["y"] };
			//	int id;
			//	int node_to_connect;
			//	int graph;
			//	for (int prop = 0; prop < object_layer.size(); prop++) {
			//		if (object_layer[obj]["properties"][prop]["name"] == "Node to Connect") {
			//			node_to_connect = object_layer[obj]["properties"][prop]["value"];
			//		}
			//		else if (object_layer[obj]["properties"][prop]["name"] == "Path Node") {
			//			id = object_layer[obj]["properties"][prop]["value"];
			//		}
			//		else if (object_layer[obj]["properties"][prop]["name"] == "Path Graph") {
			//			graph = object_layer[obj]["properties"][prop]["value"];
			//		}
			//	}
			//	if (graphs.find(graph) == graphs.end()) {
			//		graphs.insert({ graph, id });
			//	}
			//	ai->create_pathNode(pos, id, node_to_connect);
			//}
			//for (Entity e : registry.pathNodes.entities) {
			//	PathNode* path_node = registry.pathNodes.get(e);
			//	for (Entity to_link : registry.pathNodes.entities) {
			//		PathNode* node_to_link = registry.pathNodes.get(to_link);
			//		if (path_node->node_to_connect == node_to_link->id) {
			//			ai->link_nodes(path_node, node_to_link);
			//		}
			//	}
			//}
			//for (auto g : graphs) {
			//	Entity graph;
			//	for (PathNode* node : registry.pathNodes.components) {
			//		if (node->id == g.second) {
			//			registry.pathGraphs.emplace(graph, node);
			//			break;
			//		}
			//	}
			//}
		}
		else {
			// Get map array
			auto current_layer = layers[layer_num]["data"];
			// Loop through tiles in the layer
			for (int tile_index = 0; tile_index < current_layer.size(); tile_index++) {
				auto current_tile = current_layer[tile_index];
				// Check if current position has a tile
				if (current_tile != 0) {
					// Half the tilewidth/height is added because the position is based on the middle of an entity
					float pos_x = (tile_index % width) * BASE_TILE_SIZE_WIDTH + (BASE_TILE_SIZE_WIDTH / 2.0f);
					float pos_y = (tile_index / width) * BASE_TILE_SIZE_HEIGHT + (BASE_TILE_SIZE_HEIGHT / 2.0f);

					vec2 position = { pos_x, pos_y };

				if (std::find(spawns.begin(), spawns.end(), current_tile) != spawns.end()) {
					const Entity& enemySpawn = createEnemySpawn(position);
				}
				else if (std::find(item.begin(), item.end(), current_tile) != item.end()) {
					const Entity& itemSpawn = createItemSpawn(position);
				}
				else if (std::find(light.begin(), light.end(), current_tile) != light.end()) {
					const Entity& lightSource = createLightSource(position);
					LightSource& l = registry.lightSources.get(lightSource);
					l.radius = 750.0f;
				}
				else {
					// Get the index of the tileset name
					// Get the tile number of the current tile based on what it is in its own tileset
					int tile_name_index = -1;
					int tex_index = 0;
					for (int k : ts_first_gid) {
						if (current_tile >= k) {
							tex_index = current_tile.get<int>() - k;
							tile_name_index++;
						}
					}
					vec2 tileset_size = ts_sizes[tile_name_index];
					vec2 tileset_dim = ts_dimensions[tile_name_index];

						// Get texture coordinates
						int tex_x = (tex_index % (int)tileset_dim[0]) * tilewidth;
						int tex_y = (tex_index / (int)tileset_dim[0]) * tileheight;
						float left = tex_x / tileset_size[0]; // left
						float top = tex_y / tileset_size[1]; // top
						float right = (tex_x + tilewidth) / tileset_size[0];
						float bottom = (tex_y + tileheight) / tileset_size[1];
						vec2 top_left = { left, top };
						vec2 bottom_right = { right, bottom };

						// Check if tile is collidable, an entrance, or an exit
						// May want to pull this out into a helper function if more properties are introduced
						bool collidable = false;
						if (std::find(collidables.begin(), collidables.end(), current_tile) != collidables.end()) {
							collidable = true;
						}

						bool is_entrance = false;
						if (std::find(entrance.begin(), entrance.end(), current_tile) != entrance.end()) {
							is_entrance = true;
						}

						bool is_exit = false;
						if (std::find(exit.begin(), exit.end(), current_tile) != exit.end()) {
							is_exit = true;
						}

						const Entity& map_tile = createMapTile(position, (float)layer_num, top_left, bottom_right, collidable, is_entrance, is_exit, tile_name_index, map_texture_handles.data());

					}

				}
			}
		}
	}
}

void MapLoader::unloadCurrentMap(RenderSystem* renderer, AISystem* ai) {
	for (Entity e : registry.mapTiles.entities) {
		registry.remove_all_components_of(e);
	}
	for (Entity e : registry.enemySpawns.entities) {
		registry.remove_all_components_of(e);
	}
	for (Entity e : registry.itemSpawns.entities) {
		registry.remove_all_components_of(e);
	}
	//for (Entity e : registry.pathNodes.entities) {
	//	PathNode* node = registry.pathNodes.get(e);
	//	delete node;
	//	registry.pathNodes.remove(e);
	//	if (!registry.players.has(e))
	//	registry.remove_all_components_of(e);
	//}
	//for (Entity e : registry.pathGraphs.entities) {
	//	registry.remove_all_components_of(e);
	//}
	for (Entity e : registry.lightSources.entities) {
		if (!registry.players.has(e))
			registry.remove_all_components_of(e);
	}
	while (registry.items.size() > 0) registry.remove_all_components_of(registry.items.entities.back());

	renderer->unloadMapTilesets(map_texture_handles.data(), map_texture_handles.size());
}

Entity createMapTile(vec2 pos, float layer, vec2 top_left, vec2 bottom_right, bool collision, bool is_entrance, bool is_exit, int tileset_index, GLuint* handles) {
	Entity e = Entity();
	TextureInfo& textureInfo = registry.textureinfos.emplace(e);
	textureInfo.top_left = top_left;
	textureInfo.bottom_right = bottom_right;
	textureInfo.texture_id = handles[tileset_index];
	Position& position = registry.positions.emplace(e);
	position.layer = layer;
	position.scale = {BASE_TILE_SIZE_WIDTH, BASE_TILE_SIZE_HEIGHT};
	position.position = pos;
	MapTile& mapTile = registry.mapTiles.emplace(e);
	if (collision) {
		Collidable& collidable = registry.collidables.emplace(e);
		if (is_entrance) {
			Entrance& entrance = registry.entrance.emplace(e);
		}
		if (is_exit) {
			mapTile.exit = true;
		}
	}

	return e;
}

std::vector<std::string> MapLoader::getTilesetNames(json tilesets) {
	std::vector<std::string> ts_file_names;
	for (int t = 0; t < tilesets.size(); t++) {
		std::string set_name = tilesets[t]["source"];
		std::istringstream s(set_name);
		std::string name;
		std::getline(s, name, '.');
		ts_file_names.push_back(name);
	}
	return ts_file_names;
}

std::vector<int> MapLoader::getTilesetFirstGID(json tilesets) {
	std::vector<int> ts_first_gids;
	for (int t = 0; t < tilesets.size(); t++) {
		int gid = tilesets[t]["firstgid"];
		ts_first_gids.push_back(gid);
	}
	return ts_first_gids;
}


std::vector<std::vector<int>> MapLoader::getTileProperties(std::vector<std::string> ts_file_names, std::vector<int> ts_first_gid) {
	std::vector<int> collidables;
	std::vector<int> spawns;
	std::vector<int> entrance;
	std::vector<int> exit;
	std::vector<int> item;
	std::vector<int> light;
	for (int i = 0; i < ts_file_names.size(); i++) {
		std::ifstream s(std::string(PROJECT_SOURCE_DIR) + "data/textures/map/" + ts_file_names[i] + ".json");
		json tileset_json = json::parse(s);
		int firstGID = ts_first_gid[i];
		// check if they have properties
		if (tileset_json.contains("tiles")) {
			json tiles = tileset_json["tiles"];
			// get tile ids that have the specified property and add  them to vector
			for (int j = 0; j < tiles.size(); j++) {
				// collision
				if (tiles[j]["properties"][0]["value"] == true) {
					collidables.push_back(tiles[j]["id"].get<int>() + firstGID);
				}
				else if (tiles[j]["properties"][1]["value"] == true){
					spawns.push_back(tiles[j]["id"].get<int>() + firstGID);
				}
				// entrance
				if (tiles[j]["properties"][2]["value"] == true) {
					entrance.push_back(tiles[j]["id"].get<int>() + firstGID);
				}
				// exit
				else if (tiles[j]["properties"][3]["value"] == true) {
					exit.push_back(tiles[j]["id"].get<int>() + firstGID);
				}
				// item spawn
				else if (tiles[j]["properties"][4]["value"] == true) {
					item.push_back(tiles[j]["id"].get<int>() + firstGID);
				}
				else if (tiles[j]["properties"][5]["value"] == true) {
					light.push_back(tiles[j]["id"].get<int>() + firstGID);
				}
			}
		}
	}
	std::vector<std::vector<int>> results;
	results.push_back(collidables);
	results.push_back(spawns);
	results.push_back(entrance);
	results.push_back(exit);
	results.push_back(item);
	results.push_back(light);
	return results;
}

std::vector<vec2> MapLoader::getTilesetSize(std::vector<std::string> ts_file_names) {
	std::vector<vec2> tileset_sizes;
	for (int i = 0; i < ts_file_names.size(); i++) {
		std::ifstream s(std::string(PROJECT_SOURCE_DIR) + "data/textures/map/" + ts_file_names[i] + ".json");
		json tileset_json = json::parse(s);
		int tileset_height = tileset_json["imageheight"];
		int tileset_width = tileset_json["imagewidth"];
		tileset_sizes.push_back({ tileset_width, tileset_height });
		//std::cout << "x: " << tileset_width << ", y: " << tileset_height << std::endl;
	}
	return tileset_sizes;
}

std::vector<vec2> MapLoader::getTilesetDims(std::vector<std::string> ts_file_names) {
	std::vector<vec2> tileset_dims;
	for (int i = 0; i < ts_file_names.size(); i++) {
		std::ifstream s(std::string(PROJECT_SOURCE_DIR) + "data/textures/map/" + ts_file_names[i] + ".json");
		json tileset_json = json::parse(s);
		int columns = tileset_json["columns"];
		int rows = tileset_json["tilecount"].get<int>() / columns;
		tileset_dims.push_back({ columns, rows });
	}
	return tileset_dims;
}

Entity createEnemySpawn(vec2 pos) {
	const Entity& entity = Entity();
	Position& position = registry.positions.emplace(entity);
	position.position = pos;
	EnemySpawn& es = registry.enemySpawns.emplace(entity);
	return entity;
}

Entity createItemSpawn(vec2 pos) {
	const Entity& entity = Entity();
	Position& position = registry.positions.emplace(entity);
	position.position = pos;
	ItemSpawn& is = registry.itemSpawns.emplace(entity);
	return entity;
}

void MapLoader::loadNextMap(RenderSystem* renderer, AISystem* ai) {
	level += 1;
	// get random number in range [1, CITY_MAPS]
	int random_map_number = rand() % CITY_MAPS + 1;
	std::string path = "map/city_" + std::to_string(random_map_number) + ".json";
	std::cout << textures_path(path) << std::endl;

	while (textures_path(path) == current_map) {
		random_map_number = rand() % CITY_MAPS + 1;
		path = "map/city_" + std::to_string(random_map_number) + ".json";
	}

	if (level % BOSS_ROOM_LEVEL == 0) {
		path = "map/boss_room.json";
	}

	std::cout << path << "\n";
	parseMaps(textures_path(path), renderer, ai);
}

int MapLoader::getLevel() {
	return level;
}