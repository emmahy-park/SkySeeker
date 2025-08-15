#pragma once

#include "common.hpp"
#include "render_system.hpp"
#include "../tinyECS/registry.hpp"
#include "enemy_system.hpp"

class AISystem
{
private:
	static float ms_since_last_pathfind;
	static int orientation(vec2 p, vec2 q, vec2 r);
	static bool onSegment(vec2 p, vec2 q, vec2 r);
	static bool intersects_with_graph(vec2 pos_a, vec2 pos_b, PathNode* graph, std::set<PathNode*> ignore_set);

	EnemySystem* enemy_system;
	PathNode* update_node_position_edges(PathNode* node, vec2 pos, PathNode* graph, std::set<PathNode*> ignore_set);
	MoveNode& get_path_to_player(Entity& entity);
	PathNode* add_position_to_graph(Entity& entity, vec2 pos, PathNode* graph, std::set<PathNode*> ignore_set);
	void update_player_pathnode_edges();
	vec2& search_path_graph(PathNode* start, PathNode* end);
	void check_vision();
	void check_attacks();
	void find_paths();
	bool player_in_range(Entity& enemy_entity);
public:
	void step(float elapsed_ms);

	static bool does_intersect(vec2 p1, vec2 q1, vec2 p2, vec2 q2);

	AISystem();

	void init(EnemySystem* enemy_system);

	static PathNode* link_nodes(PathNode* a, PathNode* b);
	static PathNode* unlink_nodes(PathNode* a, PathNode* b);

	PathNode* create_pathNode(vec2 pos/*, int id, int node_to_connect*/);
};