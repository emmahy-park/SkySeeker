// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include <iostream>

// [7]
// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Position& position, Collidable& collidable)
{
	// abs is to avoid negative scale due to the facing direction.

	if (collidable.scale.x > 0 && collidable.scale.y > 0) {
		return { abs(collidable.scale.x), abs(collidable.scale.y) };
	}
	return { abs(position.scale.x), abs(position.scale.y) };
}

// [7] AABB Collisioon
bool collides(const Position& position1, const Position& position2, Collidable& collidable1, Collidable& collidable2)
{
	const vec2 boundingbox_1 = get_bounding_box(position1, collidable1) / 2.f;
	const vec2 boundingbox_2 = get_bounding_box(position2, collidable2) / 2.f;

	float top1 = position1.position.y + collidable1.position.y - boundingbox_1.y;
	float bottom1 = position1.position.y + collidable1.position.y + boundingbox_1.y;
	float top2 = position2.position.y + collidable2.position.y - boundingbox_2.y;
	float bottom2 = position2.position.y + collidable2.position.y + boundingbox_2.y;
	bool vertical_check = bottom1 > top2 && bottom2 > top1;

	float right1 = position1.position.x + collidable1.position.x + boundingbox_1.x;
	float left1 = position1.position.x + collidable1.position.x - boundingbox_1.x;
	float right2 = position2.position.x + collidable2.position.x + boundingbox_2.x;
	float left2 = position2.position.x + collidable2.position.x - boundingbox_2.x;
	bool horizontal_check = right1 > left2 && right2 > left1;

	if (vertical_check && horizontal_check)
		return true;
	return false;
}

void PhysicsSystem::step(float elapsed_ms)
{
	// Move each entity that has motion (invaders, projectiles, and even towers [they have 0 for velocity])
	// based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	auto& velocity_registry = registry.velocities;
	float step_seconds = elapsed_ms / 1000.f;

	for(Entity& entity : velocity_registry.entities)
	{
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// !!! TODO A1: update motion.position based on step_seconds and motion.velocity
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		Velocity& velocity = velocity_registry.get(entity);

		Position& position = registry.positions.get(entity);
		position.position += velocity.velocity * step_seconds;
	}

	auto& movement_function_registry = registry.moveFunctions;
	for (uint i = 0; i < movement_function_registry.size(); i++)
	{
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// !!! TODO A1: update motion.position based on step_seconds and motion.velocity
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		Entity entity = movement_function_registry.entities[i];
		MoveFunction& mf = movement_function_registry.components[i];


		if (!velocity_registry.has(entity)) {
			velocity_registry.emplace(entity);
		}

		Velocity& velocity = velocity_registry.get(entity);

		mf.current_time += elapsed_ms;
		if (mf.max_time != 0 && mf.current_time > mf.max_time) {
			movement_function_registry.remove(entity); // I don't like this
		}

		velocity.velocity = mf.velocity_function(mf.current_time);
	}


	// check for collisions between all collidable entities
    ComponentContainer<Collidable> &collidable_container = registry.collidables;
	for(uint i = 0; i < collidable_container.components.size(); i++)
	{
		Entity entity_i = collidable_container.entities[i];
		Position& position_i = registry.positions.get(entity_i);
		
		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		for(uint j = i+1; j < collidable_container.components.size(); j++)
		{
			Entity entity_j = collidable_container.entities[j];
			Position& position_j = registry.positions.get(entity_j);
			if (collides(position_i, position_j, collidable_container.components[i], collidable_container.components[j]))
			{				
				// Create a collisions event
				// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
				// CK: why the duplication, except to allow searching by entity_id
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				// registry.collisions.emplace_with_duplicates(entity_j, entity_i);
			}
		}
	}
}