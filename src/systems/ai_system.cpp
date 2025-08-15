#include <iostream>
#include "ai_system.hpp"
#include "world_init.hpp"
#include <cmath>
#include <set>
#include <queue>

float AISystem::ms_since_last_pathfind;

AISystem::AISystem() {
	AISystem::ms_since_last_pathfind = 0.f;
}

// Given three collinear points p, q, r, the function checks if 
// point q lies on line segment 'pr' 
bool AISystem::onSegment(vec2 p, vec2 q, vec2 r)
{
	if (q.x < max(p.x, r.x) && q.x > min(p.x, r.x) &&
		q.y < max(p.y, r.y) && q.y > min(p.y, r.y))
		return true;

	return false;
}

// To find orientation of ordered triplet (p, q, r). 
// The function returns following values 
// 0 --> p, q and r are collinear 
// 1 --> Clockwise 
// 2 --> Counterclockwise 
int AISystem::orientation(vec2 p, vec2 q, vec2 r)
{
	// See https://www.geeksforgeeks.org/orientation-3-ordered-points/ 
	// for details of below formula. 
	float val = (q.y - p.y) * (r.x - q.x) -
		(q.x - p.x) * (r.y - q.y);

	if (val == 0) return 0;  // collinear 

	return (val > 0) ? 1 : 2; // clock or counterclock wise 
}

// Returns true if line segment 'p1q1' 
// and 'p2q2' intersect. 
// Modified by Stephane Gallant to exclude the endpoints of a line segment when
// determining intersection
bool AISystem::does_intersect(vec2 p1, vec2 q1, vec2 p2, vec2 q2)
{
	// Find the four orientations needed for general and 
	// special cases 
	int o1 = orientation(p1, q1, p2);
	int o2 = orientation(p1, q1, q2);
	int o3 = orientation(p2, q2, p1);
	int o4 = orientation(p2, q2, q1);

	/*std::cout << "checking line segments: " << p1.x << "," << p1.y << "; ";
	std::cout << q1.x << "," << q1.y << "; and ";
	std::cout << p2.x << "," << p2.y << "; ";
	std::cout << q2.x << "," << q2.y << "; " << std::endl;*/

	// General case 
	// Checking for endpoint inequality causes algorithm to exclude endpoints.
	if (o1 != o2 && o3 != o4 && p1 != p2 && p1 != q2 && q1 != p2 && q1 != q2)
	{
		//std::cout << "returning true" << std::endl;
		return true;
	}

	// Special Cases 
	// p1, q1 and p2 are collinear and p2 lies on segment p1q1 
	if (o1 == 0 && onSegment(p1, p2, q1)) return true;

	// p1, q1 and q2 are collinear and q2 lies on segment p1q1 
	if (o2 == 0 && onSegment(p1, q2, q1)) return true;

	// p2, q2 and p1 are collinear and p1 lies on segment p2q2 
	if (o3 == 0 && onSegment(p2, p1, q2)) return true;

	// p2, q2 and q1 are collinear and q1 lies on segment p2q2 
	if (o4 == 0 && onSegment(p2, q1, q2)) return true;

	//std::cout << "returning false" << std::endl;
	return false; // Doesn't fall in any of the above cases 
}

void print_pathNode(PathNode& node) {
	std::cout << "Node at position: (" << node.position[0] << ", " << node.position[1] << ")";
	std::cout << "Linked to nodes at positions: " << std::endl;
	for (PathNode* pathNode : node.edges) {
		std::cout << "(" << pathNode->position[0] << ", " << pathNode->position[1] << ")" << std::endl;
	}
}

float distance(vec2 point1, vec2 point2) {
	float x = point1.x - point2.x;
	float y = point1.y - point2.y;
	return sqrtf(x * x + y * y);
}

// returns true if a line between pos_a and pos_b intersects with 
// any edge in given graph
// assumes that the graph has at least 3 nodes and each node has at least
// 2 edges each
bool AISystem::intersects_with_graph(vec2 pos_a, vec2 pos_b, PathNode* graph, std::set<PathNode*> ignore_set) {
	PathNode* currentNode = graph;
	PathNode* lastNode = nullptr;
	assert(currentNode->edges.size() > 1 && "PathNode has less than 2 edges");
	do {
		size_t i = 0;
		PathNode* nextNode = currentNode->edges[i];
		while (nextNode == lastNode || ignore_set.find(nextNode) != ignore_set.end()) {
			i += 1;
			nextNode = currentNode->edges[i];
		}
		if (nextNode != nullptr && does_intersect(pos_a, pos_b, currentNode->position, nextNode->position)) {
			//std::cout << "intersection with graph detected" << std::endl;
			return true;
		}
		lastNode = currentNode;
		currentNode = nextNode;
	} while (currentNode != graph);
	return false;
}

// assumes b exists in a.edges, and a exists in b.edges
PathNode* AISystem::unlink_nodes(PathNode* a, PathNode* b) {
	if (a == nullptr || b == nullptr) {
		return a;
	}
	for (std::vector<PathNode*>::iterator it_a = a->edges.begin(); it_a != a->edges.end(); ) {
		if (*it_a == b) {
			a->edges.erase(it_a);
			break;
		}
		it_a++;
	}
	for (std::vector<PathNode*>::iterator it_b = b->edges.begin(); it_b != b->edges.end(); ) {
		if (*it_b == a) {
			b->edges.erase(it_b);
			break;
		}
		it_b++;
	}
	return a;
}

// edges are represented as node pointers stored in the node
PathNode* AISystem::link_nodes(PathNode* a, PathNode* b) {
	//std::cout << "Linking nodes: " << std::endl;
	a->edges.push_back(b);
	//std::cout << "Node A: ";
	//print_pathNode(*a);
	b->edges.push_back(a);
	//std::cout << "Node B: ";
	//print_pathNode(*b);
	return a;
}

// current implementation uses heap memory because I (Stephane) couldn't get it to work
// with just references and the ECS. Might try refactoring for M2.
PathNode* AISystem::create_pathNode(vec2 pos/*, int node_id, int node_to_connect*/) {
	Entity entity;
	PathNode* node = new PathNode();
	registry.pathNodes.emplace(entity, node);
	node->position = pos;
	//node->id = node_id;
	//node->node_to_connect = node_to_connect;
	return node;
}

// update position of an entity's PathNode and update edges 
PathNode* AISystem::update_node_position_edges(PathNode* node, vec2 pos, PathNode* graph, std::set<PathNode*> ignore_set) {
	node->position = pos;
	for (PathNode* linkedNode : node->edges) {
		unlink_nodes(node, linkedNode);
	}
	PathNode* currentNode = graph;
	if (currentNode == nullptr) {
		return node;
	}
	PathNode* lastNode = nullptr;
	do {
		if (!intersects_with_graph(node->position, currentNode->position, graph, ignore_set)) {
			//std::cerr << "No intersection";
			link_nodes(node, currentNode);
		}
		size_t i = 0;
		PathNode* nextNode = currentNode->edges[i];
		while (nextNode == lastNode || ignore_set.find(nextNode) != ignore_set.end()) {
			nextNode = currentNode->edges[++i];
		}
		lastNode = currentNode;
		currentNode = nextNode;
	} while (currentNode != graph);

	return node;
}

// returns a pointer to a PathNode at pos, connected to graph, associated with Entity
PathNode* AISystem::add_position_to_graph(Entity& entity, vec2 pos, PathNode* graph, std::set<PathNode*> ignore_set) {
	PathNode* newNode = new PathNode();
	registry.pathNodes.emplace(entity, newNode);
	update_node_position_edges(newNode, pos, graph, ignore_set);
	return newNode;
}

// Give player a pathNode if they don't have one, update its pos and edges
void AISystem::update_player_pathnode_edges() {
	Entity& player_entity = registry.players.entities[0];
	vec2& position = registry.positions.get(player_entity).position;
	ComponentContainer<PathNode*>& pathNode_registry = registry.pathNodes;
	ComponentContainer<PathNode*>& pathGraph_registry = registry.pathGraphs;
	std::set<PathNode*> ignore_set;
	for (Entity& enemy_entity : registry.enemies.entities) {
		if (!registry.positions.has(enemy_entity)) continue;
		if (pathNode_registry.has(enemy_entity)) {
			ignore_set.insert(pathNode_registry.get(enemy_entity));
		}
	}
	PathNode* player_pathNode = nullptr;
	if (pathNode_registry.has(player_entity)) {
		player_pathNode = pathNode_registry.get(player_entity);
	}
	else {
		player_pathNode = add_position_to_graph(player_entity, position, nullptr, ignore_set);
	}
	ignore_set.insert(player_pathNode);
	for (PathNode* graph : pathGraph_registry.components) {
		update_node_position_edges(player_pathNode, position, graph, ignore_set);
	}
}

// BFS implementation. 
// todo: save path and use it as basis for new search instead of starting from scratch
vec2& AISystem::search_path_graph(PathNode* start, PathNode* end) {
	std::queue<std::vector<PathNode*>> paths_to_search;
	std::vector<PathNode*> starting_path;
	starting_path.push_back(start);
	paths_to_search.push(starting_path);
	while (paths_to_search.size() > 0) {
		std::vector<PathNode*> path_to_continue = paths_to_search.front();
		PathNode* lookup_node = path_to_continue.back();
		std::vector<PathNode*> edges_to_check = lookup_node->edges;
		for (PathNode* nextNode : edges_to_check) {
			bool onPath = false;
			for (PathNode* pathNode : path_to_continue) {
				if (pathNode == nextNode) {
					onPath = true;
					break;
				}
			}
			if (onPath) {
				continue;
			}
			else if (nextNode == end) {
				return path_to_continue[1]->position;
			}
			else {
				std::vector<PathNode*> newPath(path_to_continue);
				newPath.push_back(nextNode);
				paths_to_search.push(newPath);
			}
		}
		paths_to_search.pop();
	}
	return start->position;
}

// [5] Random / Coded Action
// Returns moveNode corresponding to the next step in the 
// shortest path from Entity to Player
MoveNode& AISystem::get_path_to_player(Entity& entity) {
	ComponentContainer<Position> position_registry = registry.positions;
	vec2& position = position_registry.get(entity).position;
	Entity& player_entity = registry.players.entities[0];
	vec2& player_position = position_registry.get(player_entity).position;

	ComponentContainer<MoveNode>& moveNode_registry = registry.moveNodes;
	if (moveNode_registry.has(entity)) {
		moveNode_registry.remove(entity);
	}
	MoveNode& new_node = moveNode_registry.emplace(entity);

	// ignore_set is populated with all pathnodes belonging to enemies and the player
	// prevents intersection checking from including their edges in its algorithm
	std::set<PathNode*> ignore_set;
	for (Entity& enemy_entity : registry.enemies.entities) {
		if (!registry.positions.has(enemy_entity)) continue;
		if (registry.pathNodes.has(enemy_entity) && enemy_entity != entity) {
			ignore_set.insert(registry.pathNodes.get(enemy_entity));
		}
	}
	ignore_set.insert(registry.pathNodes.get(player_entity));

	// If a line segment from Entity to Player does not intersect with a map obstacle edge
	// bypass the search and set Entity's movenode to the Player's position.
	for (PathNode* graph : registry.pathGraphs.components) {
		if (intersects_with_graph(position, player_position, graph, ignore_set)) {
			//std::cerr << "Intersection detected \n";
			PathNode* pathNode;
			// if the entity does not have a pathNode, give it one.
			// in either case, update its edges with map obstacle pathNodes.
			if (registry.pathNodes.has(entity)) {
				pathNode = update_node_position_edges(registry.pathNodes.get(entity), position, graph, ignore_set);
			}
			else {
				pathNode = add_position_to_graph(entity, position, graph, ignore_set);
			}

			// BFS called here
			new_node.position = search_path_graph(pathNode, registry.pathNodes.get(player_entity));
			return new_node;
		}
	}
	// no intersections detected
	new_node.position = player_position;
	return new_node;
}

// placeholder behavior
// simply sets sees_player to "true" for all enemies
void AISystem::check_vision() {
	ComponentContainer<Position> position_registry = registry.positions;
	Entity& player_entity = registry.players.entities[0];
	vec2& player_position = position_registry.get(player_entity).position;
	for (Entity& entity : registry.enemies.entities) {
		if (!registry.positions.has(entity)) continue;
		Enemy& enemy = registry.enemies.get(entity);
		vec2& position = position_registry.get(entity).position;
		if (distance(position, player_position) < BASE_TILE_SIZE_HEIGHT * 12.f){
			enemy.sees_player = true;
		}
		else {
			enemy.sees_player = false;
		}
		
	}
}

// [5] Random/Coded Action
// First updates the player pathnode's connections to existing graphs
// Then tells all enemies who can see the player to get a path
void AISystem::find_paths() {
	if (registry.players.size() > 0) {
		update_player_pathnode_edges();
		for (Entity& enemy_entity : registry.enemies.entities) {
			if (!registry.positions.has(enemy_entity)) continue;

			Enemy& enemy_component = registry.enemies.get(enemy_entity);
			if (enemy_component.sees_player && (enemy_component.current_state == ENEMY_IDLE || enemy_component.current_state == ENEMY_WALKING)) {
				get_path_to_player(enemy_entity);
			}
		}
	}
}

bool AISystem::player_in_range(Entity& enemy_entity) {
	Enemy& enemy_component = registry.enemies.get(enemy_entity);
	ComponentContainer<Position>& position_reg = registry.positions;
	vec2& enemy_position = position_reg.get(enemy_entity).position;
	vec2& player_position = position_reg.get(registry.players.entities[0]).position;
	//std::cerr << "distance: " << distance(enemy_position, player_position) << "versus range: " << enemy_component.attack_range << "\n";
	return (distance(enemy_position, player_position) <= enemy_component.attack_range);
}

void AISystem::check_attacks() {
	if (registry.players.size() > 0) {
		Position& player_pos = registry.positions.get(registry.players.entities[0]);
		for (Entity& enemy_entity : registry.enemies.entities) {
			if (!registry.positions.has(enemy_entity)) continue;

			Enemy& enemy_component = registry.enemies.get(enemy_entity);
			if (enemy_component.current_state != ENEMY_IDLE && enemy_component.current_state != ENEMY_WALKING) {
				continue;
			}
			if (enemy_component.sees_player && player_in_range(enemy_entity)) {
				//std::cerr << "player detected by enemy";
				enemy_system->prepare_attack(enemy_entity, player_pos.position);
			}
		}
	}
}

void AISystem::step(float elapsed_ms)
{
	AISystem::ms_since_last_pathfind += elapsed_ms;
	check_vision();
	check_attacks();

	// each node corresponds to a corner of the obstacle
	// linking nodes creates an edge, corresponding to the sides of the obstacle
	//if (registry.pathGraphs.size() == 0) {
	//	vec2 position0 = { 190, 260 };
	//	vec2 position1 = { 640, 260};
	//	vec2 position2 = { 640, 420 };
	//	vec2 position3 = { 190, 420 };
	//	PathNode* node0 = create_pathNode(position0);
	//	PathNode* node1 = create_pathNode(position1);
	//	PathNode* node2 = create_pathNode(position2);
	//	PathNode* node3 = create_pathNode(position3);
	//	link_nodes(node0, node1);
	//	link_nodes(node1, node2);
	//	link_nodes(node2, node3);
	//	link_nodes(node3, node0);
	//	Entity graph_entity;
	//	registry.pathGraphs.emplace(graph_entity, node0);
	//}
	if (AISystem::ms_since_last_pathfind >= AI_PATHFINDING_INTERVAL_MS) {
		find_paths();
		AISystem::ms_since_last_pathfind = 0.f;
	}
}

void AISystem::init(EnemySystem* enemy_system) {
	this->enemy_system = enemy_system;
}