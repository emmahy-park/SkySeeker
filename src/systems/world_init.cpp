#include "world_init.hpp"
#include "tinyECS/registry.hpp"
#include "../data/items.hpp"
#include "particle_system.hpp"

#include <iostream>
#include <math.h>



// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! TODO A1: implement grid lines as gridLines with renderRequests and colors
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
Entity createGridLine(vec2 start_pos, vec2 end_pos)
{
	Entity entity = Entity();

	// TODO A1: create a gridLine component

	// re-use the "DEBUG_LINE" renderRequest
	/*
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::EGG,
			GEOMETRY_BUFFER_ID::DEBUG_LINE
		}
	);
	*/

	// TODO A1: grid line color (choose your own color)

	return entity;
}

Entity createTower(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();

	// new tower
	auto& t = registry.towers.emplace(entity);
	t.range = (float)WINDOW_WIDTH_PX / (float)GRID_CELL_WIDTH_PX;
	t.timer_ms = TOWER_TIMER_MS;	// arbitrary for now

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& pos = registry.positions.emplace(entity);
	pos.position = position;

	std::cout << "INFO: tower position: " << position.x << ", " << position.y << std::endl;

	// Setting initial values, scale is negative to make it face the opposite way
	pos.scale = vec2({ -TOWER_BB_WIDTH, TOWER_BB_HEIGHT });
	pos.angle = 180.0f;

	auto& textureinfo = registry.textureinfos.emplace(entity);
	textureinfo.texture_id = renderer->getTextureHandle(TEXTURE_ASSET_ID::TOWER);

	// Create collidable
	registry.collidables.emplace(entity);

	return entity;
}

void removeTower(vec2 position) {
	// remove any towers at this position
	for (Entity& tower_entity : registry.towers.entities) {
		// get each tower's position to determine it's row
		const Position& tower_position = registry.positions.get(tower_entity);
		
		if (tower_position.position.y == position.y) {
			// remove this tower
			registry.remove_all_components_of(tower_entity);
			std::cout << "tower removed" << std::endl;
		}
	}
}

Entity createLine(vec2 position, vec2 scale)
{
	Entity entity = Entity();

	// Create motion
	Position& pos = registry.positions.emplace(entity);
	pos.position = position;
	pos.scale = scale;

	registry.debugComponents.emplace(entity);
	return entity;
}

Entity createProjectile(Entity& owner, vec2 projectile_owner_position, vec2 target_position, float range, float speed, vec2 size, float damage_multiplier, GLuint texture_handle) {
	// reserve an entity
	Entity entity = Entity();

	// add Projectile entity
	Projectile& projectile = registry.projectiles.emplace(entity);
	projectile.damage_multiplier = damage_multiplier;
	projectile.range = range;
	projectile.initial_position = projectile_owner_position;

	// calculate direction
	vec2 direction = normalize(target_position - projectile_owner_position);

	// attacker
	Attack& attack = registry.attacks.emplace(entity);
	attack.attacker = owner;

	// initialize Position
	Position& position = registry.positions.emplace(entity);
	position.position = projectile_owner_position;
	position.layer = PROJECTILE_LAYER;
	position.scale = size;
	position.angle = 0.f;

	// velocity
	Velocity& velocity = registry.velocities.emplace(entity);
	velocity.velocity = speed * direction;

	// colllidable
	registry.collidables.emplace(entity);

	// texture
	registry.textureinfos.emplace(entity);
	TextureInfo& projectile_texture = registry.textureinfos.get(entity);
	projectile_texture.texture_id = texture_handle;

	LightSource& ls = registry.lightSources.emplace(entity);
	ls.radius = size.x + 200.f;

	return entity;
}

Entity createItem(ItemInfo info, bool is_pickup) {
	Entity e = Entity();

	Item& i = registry.items.emplace(e);

	i.info = info;
	i.is_pickup = is_pickup;

	TextureInfo& ti = registry.textureinfos.emplace(e);
	ti.texture_id = item_handles[info.texture_name];

	return e;
}

Entity createCollidableItem(ItemInfo info, vec2 position) {
	Entity e = createItem(info, false);

	Position& p = registry.positions.emplace(e);
	p.position = position;
	p.scale = ITEM_SIZE;
	p.layer = 2; // In front

	Collidable& c = registry.collidables.emplace(e);
	c.scale = ITEM_SIZE * 2.f; // double radius?

	MoveFunction& mf = registry.moveFunctions.emplace(e);
	mf.velocity_function = [](float time) {
		// Time in ms
		// Period should be 
		float period = 2000; // 2 seconds

		float x = 2 * M_PI * time / period;
		return vec2(0, sin(x) * BASE_TILE_SIZE_HEIGHT / 2);
	};

	return e;
}

Entity createHealthBar(RenderSystem* renderer) {
	Entity e = Entity();

	HealthBar& health_bar = registry.healthbar.emplace(e);
	Player& player = registry.players.components[0];

	health_bar.health = player.base_max_health;

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(e, &mesh);
	
	Position& p = registry.positions.emplace(e);
	p.position = vec2(HEALTHBAR_XPOS, HEALTHBAR_YPOS);
	p.layer = 1.0f;
	p.scale = { health_bar.health * 2, 20 };
	p.position.x += p.scale.x / 2;

	TextureInfo& texture = registry.textureinfos.emplace(e);

	return e;
}

Entity createLightSource(vec2 position) {
	Entity e = Entity();

	LightSource& ls = registry.lightSources.emplace(e);

	Position& p = registry.positions.emplace(e);
	p.position = position;
	return e;
}
