#pragma once
#include <vector>

#include "tiny_ecs.hpp"
#include "components.hpp"

class ECSRegistry
{
	// callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface*> registry_list;

public:
	// Manually created list of all components this game has
	// TODO: A1 add a LightUp component
	ComponentContainer<DeathTimer> deathTimers;
	ComponentContainer<Collision> collisions;
	ComponentContainer<Collidable> collidables;
	ComponentContainer<Velocity> velocities;
	ComponentContainer<TextureInfo> textureinfos;
	ComponentContainer<Position> positions;
	ComponentContainer<Animation> animations;
	ComponentContainer<Living> livings;
	ComponentContainer<Player> players;
	ComponentContainer<MapTile> mapTiles;
	ComponentContainer<Attack> attacks;
	ComponentContainer<Mesh*> meshPtrs;
	ComponentContainer<ScreenState> screenStates;
	ComponentContainer<DebugComponent> debugComponents;
	ComponentContainer<vec3> colors;
	ComponentContainer<Tower> towers;
	ComponentContainer<GridLine> gridLines;
	ComponentContainer<Enemy> enemies;
	ComponentContainer<MoveNode> moveNodes;
	ComponentContainer<PathNode*> pathNodes;
	ComponentContainer<PathNode*> pathGraphs;
	ComponentContainer<Projectile> projectiles;
	ComponentContainer<Sword> swords;
	ComponentContainer<Entrance> entrance;
	ComponentContainer<EnemySpawn> enemySpawns;
	ComponentContainer<ItemSpawn> itemSpawns;
	ComponentContainer<Font> fonts;
	ComponentContainer<Item> items;
	ComponentContainer<MoveFunction> moveFunctions;
	ComponentContainer<GameState> gameStates;
	ComponentContainer<TriangleMesh> triangleMesh;
	ComponentContainer<HealthBar> healthbar;
	ComponentContainer<Particle> particles;
	ComponentContainer<Parry> parries;
	ComponentContainer<LightSource> lightSources;

	// constructor that adds all containers for looping over them
	ECSRegistry()
	{
		// TODO: A1 add a LightUp component
		registry_list.push_back(&deathTimers);
		registry_list.push_back(&positions);
		registry_list.push_back(&velocities);
		registry_list.push_back(&textureinfos);
		registry_list.push_back(&animations);
		registry_list.push_back(&livings);
		registry_list.push_back(&collisions);
		registry_list.push_back(&mapTiles);
		registry_list.push_back(&attacks);
		registry_list.push_back(&players);
		registry_list.push_back(&meshPtrs);
		registry_list.push_back(&screenStates);
		registry_list.push_back(&debugComponents);
		registry_list.push_back(&colors);
		registry_list.push_back(&towers);
		registry_list.push_back(&gridLines);
		registry_list.push_back(&enemies);
		registry_list.push_back(&moveNodes);
		registry_list.push_back(&pathNodes);
		registry_list.push_back(&pathGraphs);
		registry_list.push_back(&projectiles);
		registry_list.push_back(&collidables);
		registry_list.push_back(&swords);
		registry_list.push_back(&entrance);
		registry_list.push_back(&enemySpawns);
		registry_list.push_back(&itemSpawns);
		registry_list.push_back(&items);
		registry_list.push_back(&moveFunctions);
		registry_list.push_back(&gameStates);
		registry_list.push_back(&triangleMesh);
		registry_list.push_back(&healthbar);
		registry_list.push_back(&particles);
		registry_list.push_back(&parries);
		registry_list.push_back(&lightSources);
	}

	void clear_all_components() {
		for (ContainerInterface* reg : registry_list)
			reg->clear();
	}

	void list_all_components() {
		printf("Debug info on all registry entries:\n");
		for (ContainerInterface* reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e) {
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface* reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e) {
		for (ContainerInterface* reg : registry_list)
			reg->remove(e);
	}
};

extern ECSRegistry registry;