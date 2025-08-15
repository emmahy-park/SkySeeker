#include "player_system.hpp"
#include "../config.hpp"
#include "../util/util.hpp"
#include "world_init.hpp"
#include "../data/items.hpp"
#include <iostream>


void PlayerSystem::changeState(Entity e, PLAYER_STATES state) {
	Player& p = registry.players.get(e);
	Animation& a = registry.animations.get(e);

	assert(state < PLAYER_STATE_COUNT);

	override_texture = false;

	p.current_state = state;
	a.current_frame = 0; // reset frames
	if (state < num_animations) // otherwise should be dealt with in switch
		a.max_frames = num_player_animations[(int)state];

	// Setup animation
	switch (state) {
		case PLAYER_IDLE:
			a.type = LINEAR_ANIMATION;
			a.info.la.animation_step = 200.0f; // 200 ms
			a.info.la.animation_ms = 0.0f;
			break;
		case PLAYER_ATTACKING:
		{
			a.type = SINGLE_LINEAR_ANIMATION;

			float swing_time = 1000.0f / BASE_SWING_SPEED;

			a.info.sla.animation_step = swing_time / a.max_frames;
			a.info.sla.animation_ms = 0.0f;
			a.info.sla.callback_fn = [](Entity e) {
				Player& p = registry.players.components[0];
				p.current_state = PLAYER_STATE_COUNT; // use as a reset state
				};
			break;
		}
		case PLAYER_DASHING: // TODO: Placeholder
		{
			a.type = SINGLE_LINEAR_ANIMATION;

			float dash_time = BASE_DASH_TIME;

			a.info.sla.animation_step = dash_time / a.max_frames;
			a.info.sla.animation_ms = 0.0f;
			a.info.sla.callback_fn = [](Entity e) {
				Player& p = registry.players.components[0];
				p.current_state = PLAYER_STATE_COUNT; // use as a reset state
			};
			break;
		}
		case PLAYER_CHARGING: // use normal walking TODO: find texture
			override_texture = true;
			override_texture_state = PLAYER_WALKING;
			a.max_frames = num_player_animations[PLAYER_WALKING];

			a.type = LINEAR_ANIMATION;
			a.info.la.animation_step = 200.0f; // 200 ms
			a.info.la.animation_ms = 0.0f;
			break;
		default: // by default use this
			a.type = LINEAR_ANIMATION;
			a.info.la.animation_step = 200.0f; // 200 ms
			a.info.la.animation_ms = 0.0f;
			break;
	}
}

// Initializes player sprites and texture info
void PlayerSystem::init(RenderSystem& renderer) {

	GLuint handles[3];
	std::string paths[] = { SPRITE_PATH, SWORD_PATH, PROJECTILE_PATH };

	// Load textures
	renderer.loadGlTextures(handles, paths, sizeof(handles) / sizeof(GLuint));

	// This unfortunately needs to happen because there are no arrays of references
	player_handle = handles[0];
	sword_handle = handles[1];
	projectile_handle = handles[2];

	// Initialize texture_coords
	float vertical_size = 1.0f / num_animations; // vertical size
	float horizontal_size = 1.0f / MAX_PLAYER_ANIMATIONS;

	for (int i = 0; i < num_animations; i++) {
		texture_coords[i] = std::vector<TextureCoords>(num_player_animations[i], TextureCoords());
		for (int j = 0; j < num_player_animations[i]; j++) {
			TextureCoords& tc = texture_coords[i][j];
			
			tc.top_left = vec2(j * horizontal_size, i * vertical_size);
			tc.bottom_right = vec2((j + 1) * horizontal_size, (i + 1) * vertical_size);
		}
	}
}

void PlayerSystem::updateMovement(Entity player_entity) {
	Player& player = registry.players.get(player_entity);

	if (player.current_state == PLAYER_DASHING) { // TODO: Placeholder
		return; // dashing is handled on its own/can be edited here
	}

	float vertical = (float)(movement_pressed[(int)DIRECTIONS::SOUTH] - movement_pressed[(int)DIRECTIONS::NORTH]);
	float horizontal = (float)(movement_pressed[(int)DIRECTIONS::EAST] - movement_pressed[(int)DIRECTIONS::WEST]);

	if (vertical != 0 && horizontal != 0) { // diagonal movement
		vertical *= ONE_OVER_ROOT_TWO;
		horizontal *= ONE_OVER_ROOT_TWO;
	}

	if (player.current_state != PLAYER_ATTACKING) {
		if (horizontal < 0) {
			flipped = true;
		}
		else if (horizontal > 0) { // if 0 then don't change flipped status
			flipped = false;
		}
	}

	if (is_moving() && player.current_state == PLAYER_IDLE) { // should change out of idle
		changeState(player_entity, PLAYER_WALKING);
	} else if (!is_moving() && player.current_state == PLAYER_WALKING) { // should change out of walking
		changeState(player_entity, PLAYER_IDLE);
	}

	assert(registry.players.size() == 1);
	
	Velocity& velocity = registry.velocities.get(player_entity);
	

	velocity.velocity = vec2(horizontal, vertical) * player.current_speed;
}

// [2] Sprite Animation
void PlayerSystem::updateTexture(Entity player_entity) {
	Position& pos = registry.positions.get(player_entity);
	Player& player = registry.players.get(player_entity);
	// Set texture info
	TextureInfo& ti = registry.textureinfos.get(player_entity);
	Animation& a = registry.animations.get(player_entity);

	pos.scale = flipped ? vec2(-SPRITE_SIZE.x, SPRITE_SIZE.y) : SPRITE_SIZE;

	PLAYER_STATES texture_state = player.current_state;

	if (override_texture) {
		texture_state = override_texture_state;
	}

	TextureCoords& tc = texture_coords[texture_state][a.current_frame];

	ti.top_left = tc.top_left;
	ti.bottom_right = tc.bottom_right;
}

void PlayerSystem::step(float elapsed_ms) {
	Entity& player_entity = registry.players.entities[0];
	Player& p = registry.players.components[0];
	if (p.current_state == PLAYER_STATE_COUNT) changeState(player_entity, PLAYER_IDLE);

	swing_swords(elapsed_ms, player_entity);
	step_parry(elapsed_ms);

	updateMovement(player_entity);
	updateTexture(player_entity);

	if (p.dash_timer > 0) {
		p.dash_timer -= elapsed_ms;
	} 

	if (p.range_timer > 0) {
		p.range_timer -= elapsed_ms;
	}

	if (p.special_timer > 0) {
		p.special_timer -= elapsed_ms;
	}

	if (p.current_state == PLAYER_CHARGING) {
		p.charge_time += elapsed_ms;
	}
}

bool PlayerSystem::is_moving() {
	return movement_pressed[(int)DIRECTIONS::NORTH] || movement_pressed[(int)DIRECTIONS::EAST]
		|| movement_pressed[(int)DIRECTIONS::SOUTH] || movement_pressed[(int)DIRECTIONS::WEST];
}

void PlayerSystem::handle_dash() {

	assert(registry.players.size() == 1);
	Entity& player_entity = registry.players.entities[0];
	Player& player = registry.players.get(player_entity);
	if (player.dash_timer > 0) return; // Cannot dash


	float vertical = (float)(movement_pressed[(int)DIRECTIONS::SOUTH] - movement_pressed[(int)DIRECTIONS::NORTH]);
	float horizontal = (float)(movement_pressed[(int)DIRECTIONS::EAST] - movement_pressed[(int)DIRECTIONS::WEST]);
	if (vertical == 0 && horizontal == 0) return; // No directions to dash


	if (vertical != 0 && horizontal != 0) { // diagonal movement
		vertical *= ONE_OVER_ROOT_TWO;
		horizontal *= ONE_OVER_ROOT_TWO;
	}


	if (player.current_state != PLAYER_ATTACKING) {
		if (horizontal < 0) {
			flipped = true;
		}
		else if (horizontal > 0) { 
			flipped = false;
		}
		// if 0 then don't change flipped status
	}

	player.dash_timer = BASE_DASH_COOLDOWN;
	
	Living& living = registry.livings.get(player_entity);

	if (living.invincible_time < BASE_DASH_TIME) { // Only update invincibility time if it will give more
		living.invincible_time = BASE_DASH_TIME;
	}
	

	changeState(player_entity, PLAYER_DASHING); // TODO: Placeholder


	Velocity& velocity = registry.velocities.get(player_entity);
	velocity.velocity = vec2(horizontal, vertical) * player.current_speed * DASH_SPEED_MODIFIER;

}

// [4] Player Movement: WASD
void PlayerSystem::on_key(int key, int action, int mod) {

	Entity& player_entity = registry.players.entities[0];
	Player& player = registry.players.get(player_entity);
	
	if (action == GLFW_PRESS) {
		if (key == config.key_bindings.move_up) movement_pressed[(int)DIRECTIONS::NORTH] = true;
		else if (key == config.key_bindings.move_down) movement_pressed[(int)DIRECTIONS::SOUTH] = true;
		else if (key == config.key_bindings.move_right) movement_pressed[(int)DIRECTIONS::EAST] = true;
		else if (key == config.key_bindings.move_left) movement_pressed[(int)DIRECTIONS::WEST] = true;
		else if (key == config.key_bindings.dash) handle_dash();
	}
	else if (action == GLFW_RELEASE) {
		if (key == config.key_bindings.move_up) movement_pressed[(int)DIRECTIONS::NORTH] = false;
		else if (key == config.key_bindings.move_down) movement_pressed[(int)DIRECTIONS::SOUTH] = false;
		else if (key == config.key_bindings.move_right) movement_pressed[(int)DIRECTIONS::EAST] = false;
		else if (key == config.key_bindings.move_left) movement_pressed[(int)DIRECTIONS::WEST] = false;
	}
}

void PlayerSystem::update_base_values(Player& player, Living& living) {
	player.base_speed = BASE_SPEED;
	player.base_attack = living.attack;
	player.base_special_attack = living.special_attack;
	player.base_max_health = living.max_health;
	player.base_defense = living.defense;
	player.base_crit_rate = living.crit_rate;
	player.base_crit_damage = living.crit_damage;
}

void PlayerSystem::update_stats(Player& player, Living& living) {

	//living.attack = (player.base_attack + player.attack_bonus.flat_bonus) * player.attack_bonus.percent_bonus; TODO: This is handled in deal_damage (change?)

	float current_health_percent = living.current_health / living.max_health;

	player.current_speed = apply_bonus(player.base_speed, player.speed_bonus);
	living.max_health = apply_bonus(player.base_max_health, player.health_bonus);
	living.current_health = living.max_health * current_health_percent;
}

Entity PlayerSystem::create_player(RenderSystem& renderer, std::vector<UpgradeInfo> current_upgrades) {
	const Entity& player = Entity();
	auto& pos = registry.positions.emplace(player);
	// TODO: Get starting position and have a proper scale
	pos.position = Util::get_position_from_coord(vec2(2, 2)); // Don't know where center is. Should be changed on map load
	pos.scale = SPRITE_SIZE;
	pos.layer = 1.0f;

	auto& collidable = registry.collidables.emplace(player);
	collidable.position = COLLISION_OFFSET;
	collidable.scale = COLLISION_SIZE;

	auto& velocity = registry.velocities.emplace(player);
	velocity.velocity = vec2(0, 0);

	auto& texture_info = registry.textureinfos.emplace(player);
	texture_info.texture_id = player_handle;

	auto& animation = registry.animations.emplace(player);

	auto& player_component = registry.players.emplace(player);
	auto& living = registry.livings.emplace(player);

	auto& light = registry.lightSources.emplace(player);
	light.radius = 250;

	update_base_values(player_component, living);
	player_component.current_speed = player_component.base_speed;

	this->player_entity = player;
	this->player = &player_component;

	apply_upgrades(current_upgrades);

	changeState(player, PLAYER_IDLE);

	return player;
}

// [4] create sword mesh based on given direction
void create_sword_mesh(vec2 swing_direction, Entity sword_entity, Position& sword_position) {
	vec2 swing_direction_perpendicular = { -swing_direction.y, swing_direction.x };
	vec2 size = sword_position.scale;
	float sword_half_width = size.x / 10.0f;
	float sword_half_length = size.y / 2.0f;
	float sword_quarter_length = sword_half_length / 2.0f;
	float sword_quarter_width = sword_half_width / 2.0f;
	vec2 bot_right = sword_position.position + sword_half_width * swing_direction_perpendicular + sword_half_length * swing_direction;
	vec2 bot_left = sword_position.position - sword_half_width * swing_direction_perpendicular + sword_half_length * swing_direction;
	vec2 top_right = sword_position.position + sword_half_width * swing_direction_perpendicular - sword_half_length * swing_direction;
	vec2 top_left = sword_position.position - sword_half_width * swing_direction_perpendicular - sword_half_length * swing_direction;
	vec2 middle_right = sword_position.position + sword_half_width * swing_direction_perpendicular;
	vec2 middle_left = sword_position.position - sword_half_width * swing_direction_perpendicular;

	vec2 bot_quarter_right = sword_position.position + sword_half_width * swing_direction_perpendicular + sword_quarter_length * swing_direction;
	vec2 bot_quarter_left = sword_position.position - sword_half_width * swing_direction_perpendicular + sword_quarter_length * swing_direction;
	vec2 top_quarter_right = sword_position.position + sword_half_width * swing_direction_perpendicular - sword_quarter_length * swing_direction;
	vec2 top_quarter_left = sword_position.position - sword_half_width * swing_direction_perpendicular - sword_quarter_length * swing_direction;

	vec2 tip_bot_right = sword_position.position + sword_quarter_width * swing_direction_perpendicular - sword_quarter_length * swing_direction;
	vec2 tip_bot_left = sword_position.position - sword_quarter_width * swing_direction_perpendicular - sword_quarter_length * swing_direction;
	vec2 tip_top_right = sword_position.position + sword_quarter_width * swing_direction_perpendicular - sword_half_length * swing_direction;
	vec2 tip_top_left = sword_position.position - sword_quarter_width * swing_direction_perpendicular - sword_half_length * swing_direction;

	vec2 handle_bot_right = sword_position.position + sword_quarter_width * swing_direction_perpendicular + sword_half_length * swing_direction;
	vec2 handle_bot_left = sword_position.position - sword_quarter_width * swing_direction_perpendicular + sword_half_length * swing_direction;
	vec2 handle_top_right = sword_position.position + sword_quarter_width * swing_direction_perpendicular + sword_quarter_length * swing_direction;
	vec2 handle_top_left = sword_position.position - sword_quarter_width * swing_direction_perpendicular + sword_quarter_length * swing_direction;

	TriangleMesh& triangle_meshes = registry.triangleMesh.get(sword_entity);
	
	if (triangle_meshes.triangle_mesh.size() > 0) {
		triangle_meshes.triangle_mesh.clear(); // clear all existing triangles for sword
	}

	Triangle first_quarter(top_quarter_left, top_quarter_right, middle_right); // create new triangles
	Triangle second_quarter(top_quarter_left, middle_left, middle_right);
	Triangle third_quarter(middle_left, middle_right, bot_quarter_right);
	Triangle last_quarter(middle_left, bot_quarter_left, bot_quarter_right);

	Triangle tip_first_half(tip_top_left, tip_top_right, tip_bot_right);
	Triangle tip_second_half(tip_top_left, tip_bot_left, tip_bot_right);
	Triangle handle_first_half(handle_top_left, handle_top_right, handle_bot_right);
	Triangle handle_second_half(handle_top_left, handle_bot_left, handle_bot_right);

	triangle_meshes.triangle_mesh.push_back(first_quarter); // add to sword's triangle mesh
	triangle_meshes.triangle_mesh.push_back(second_quarter);
	triangle_meshes.triangle_mesh.push_back(third_quarter);
	triangle_meshes.triangle_mesh.push_back(last_quarter);

	triangle_meshes.triangle_mesh.push_back(tip_first_half);
	triangle_meshes.triangle_mesh.push_back(tip_second_half);
	triangle_meshes.triangle_mesh.push_back(handle_first_half);
	triangle_meshes.triangle_mesh.push_back(handle_second_half);
}

void PlayerSystem::swing_swords(float elapsed_ms, Entity& sword_owner_entity) {

	// Swing each sword entity
	auto& sword_registry = registry.swords;

	// get player position
	vec2 player_position = registry.positions.get(sword_owner_entity).position;
	//std::cout << player_position[0] << " " << player_position[1] << std::endl;
	//for (uint i = 0; i < sword_registry.size(); i++)
	for (Entity sword_entity : sword_registry.entities)
	{
		Sword& sword = sword_registry.get(sword_entity);
		float step_seconds = elapsed_ms / 1000.f;

		Position& sword_position = registry.positions.get(sword_entity);

		// advance sword swing with circular motion
		sword.swing_progress += sword.swing_speed * step_seconds;

		// M1[3] interpolation implementation
		// interpolate sword arc movement
		
		// calculate start and end tangents
		float m0 = 0.5f * (sword.start_angle - sword.end_angle);
		float m1 = 0.5f * (sword.start_angle - sword.end_angle);
		sword.angle = hermite(sword.swing_progress, sword.start_angle, sword.end_angle, m0, m1);

		// cos and sin take in angle in radians
		sword_position.position[0] = sword.radius * cos(sword.angle) + player_position[0];
		sword_position.position[1] = sword.radius * sin(sword.angle) + player_position[1];

		// rotate sword position angle
		sword_position.angle = sword.angle * 180.0f / M_PI + 45.0f;

		// move each vertex in sword's mesh
		TriangleMesh& sword_mesh = registry.triangleMesh.get(sword_entity);
		for (Triangle& triangle : sword_mesh.triangle_mesh) {
			for (vec2& vertex : triangle.vertices) {
				
				// distance from vertex to player
				float distance_to_player = distance(vertex, player_position);

				float rotated_x = distance_to_player * cos(sword.angle);
				float rotated_y = distance_to_player * sin(sword.angle);

				// add rotated vertex to player position
				vertex.x = rotated_x + player_position.x;
				vertex.y = rotated_y + player_position.y;
			}
		}
		
		// remove sword if swing has reached end
		if (sword.swing_progress >= 1.0) {
			registry.remove_all_components_of(sword_entity);
		}
	}

	// decrement sword swing timer
	Entity& player_entity = registry.players.entities[0];
	Player& player = registry.players.get(player_entity);
	player.sword_timer -= elapsed_ms;
}

Entity PlayerSystem::createSwordSwing(vec2 sword_owner_position, vec2 mouse_position, float swing_angle, float swing_speed, float layer, float radius, vec2 size) {

	// convert swing_angle from degrees to radians
	float swing_radians = swing_angle * M_PI / 180.0f;

	// reserve an entity
	Entity entity = Entity();

	// calculate sword position
	vec2 swing_direction = normalize(mouse_position - sword_owner_position);
	vec2 sword_position = sword_owner_position + swing_direction * radius;

	// add Sword entity
	Sword& sword = registry.swords.emplace(entity);
	sword.radius = distance(sword_owner_position, sword_position);
	sword.mouse_angle = atan2(swing_direction.y, swing_direction.x);
	sword.start_angle = sword.mouse_angle - (swing_radians / 2);
	sword.end_angle = sword.mouse_angle + (swing_radians / 2);
	sword.swing_progress = 0.0;
	sword.swing_speed = swing_speed;

	Attack& attack = registry.attacks.emplace(entity);
	attack.attacker = registry.players.entities[0];

	// initialize Position
	Position& position = registry.positions.emplace(entity);
	position.position = sword_position;
	position.layer = layer;
	position.scale = size;
	position.angle = 0.f;

	// sword should be collidable
	Collidable& collidable = registry.collidables.emplace(entity);
	collidable.scale = size * ROOT_TWO; // Biggest possible size of the sword

	// std::cout << "Sword initial position: " << sword_position[0] << " " << sword_position[1] << std::endl;
	// std::cout << sword.radius << std::endl;

	// create mesh
	TriangleMesh& triangle_meshes = registry.triangleMesh.emplace(entity);
	create_sword_mesh(swing_direction, entity, position);

	registry.textureinfos.emplace(entity);
	TextureInfo& sword_texture = registry.textureinfos.get(entity);
	sword_texture.texture_id = sword_handle;

	return entity;
}

Entity PlayerSystem::createParry(vec2 parry_owner_position, vec2 mouse_position, float parry_time_window, float layer, float radius, vec2 size) {
	Entity entity = Entity();

	Parry& parry = registry.parries.emplace(entity);
	parry.parry_time_window = parry_time_window;

	Attack& attack = registry.attacks.emplace(entity);
	attack.attacker = registry.players.entities[0];

	// calculate parry position
	vec2 parry_direction = normalize(mouse_position - parry_owner_position);
	vec2 parry_direction_perpendicular = { -parry_direction.y, parry_direction.x };
	vec2 parry_position = parry_owner_position + parry_direction * radius;

	std::cout << parry_position[0] << ", " << parry_position[1] << std::endl;

	// initialize Position
	Position& position = registry.positions.emplace(entity);
	position.position = parry_position;
	position.layer = layer;
	position.scale = size;
	position.angle = atan2(parry_direction.y, parry_direction.x) * 180.0f / M_PI + 45.0f;

	Velocity& velocity = registry.velocities.emplace(entity);
	velocity.velocity = { 0.0, 0.0 };

	// calculate parry mesh
	float parry_half_width = size.x / 2.0f;
	float parry_half_length = size.x / 2.0f;
	vec2 bot_right = parry_position + parry_half_width * parry_direction_perpendicular + parry_half_length * parry_direction;
	vec2 bot_left = parry_position - parry_half_width * parry_direction_perpendicular + parry_half_length * parry_direction;
	vec2 top_right = parry_position + parry_half_width * parry_direction_perpendicular - parry_half_length * parry_direction;
	vec2 top_left = parry_position - parry_half_width * parry_direction_perpendicular - parry_half_length * parry_direction;

	const Triangle& triangle_1 = Triangle(bot_left, bot_right, top_right);
	const Triangle& triangle_2 = Triangle(top_right, top_left, bot_left);

	TriangleMesh& parry_mesh = registry.triangleMesh.emplace(entity);
	parry_mesh.triangle_mesh.push_back(triangle_1);
	parry_mesh.triangle_mesh.push_back(triangle_2);

	// bounding box should be collidable
	Collidable& collidable = registry.collidables.emplace(entity);
	collidable.scale = size;

	// texture
	registry.textureinfos.emplace(entity);
	TextureInfo& parry_texture = registry.textureinfos.get(entity);
	parry_texture.texture_id = sword_handle;

	return entity;
}

void PlayerSystem::start_charging() {
	if (!can_ranged_attack()) return;

	player->charge_time = 0.f;
	player->range_timer = BASE_RANGED_ATTACK_PERIOD;
	changeState(player_entity, PLAYER_CHARGING);
}

bool PlayerSystem::do_ranged_attack(vec2 mouse_position) {

	Entity& player_entity = registry.players.entities[0];
	Player& player = registry.players.get(player_entity);

	if (player.current_state != PLAYER_CHARGING) return false;

	float charge_percent = min(player.charge_time / MAX_CHARGE_TIME, 1.0f);

	vec2 player_position = registry.positions.get(player_entity).position;
	float range = BASE_RANGED_ATTACK_RANGE * (1 + charge_percent);
	float speed = BASE_RANGED_ATTACK_SPEED * (1 + charge_percent);
	vec2 size = BASE_RANGED_ATTACK_SIZE * (1 + charge_percent); // size of projectile

	createProjectile(player_entity, player_position, mouse_position, range, speed, size, (1 + charge_percent), projectile_handle);

	changeState(player_entity, PLAYER_IDLE);

	return true;
}

bool PlayerSystem::can_ranged_attack() {
	return player->range_timer <= 0 && (player->current_state == PLAYER_IDLE || player->current_state == PLAYER_WALKING);
}

bool PlayerSystem::can_melee_attack() {
	return player->sword_timer <= 0 && (player->current_state == PLAYER_IDLE || player->current_state == PLAYER_WALKING);
}

bool PlayerSystem::can_special() {
	return player->special_timer <= 0 && (player->current_state == PLAYER_IDLE || player->current_state == PLAYER_WALKING);
}

void PlayerSystem::do_melee_attack(vec2 mouse_position) {
	if (!can_melee_attack()) return;

	Position& player_position = registry.positions.get(player_entity);
	float radius = 50.f;
	vec2 size = { 50.f, 50.f }; // size of sword
	float swing_angle = 90.0;
	float swing_speed = BASE_SWING_SPEED;
	Entity sword_entity = createSwordSwing(player_position.position, mouse_position, swing_angle, swing_speed, 1, radius, size);

	player->sword_timer = 500.0;
	if (player_position.position.x > mouse_position.x) flipped = true;
	else flipped = false;

	changeState(player_entity, PLAYER_ATTACKING);
}

void PlayerSystem::do_special(vec2 mouse_position) {
	if (!can_special()) return;

	Entity player_entity = registry.players.entities[0];
	SPECIAL_STATES player_special = registry.players.get(player_entity).current_special;

	switch (player_special) {
	case SPECIAL_PARRY:
		Position& player_position = registry.positions.get(player_entity);
		vec2 size = { 40.f, 40.f }; // bounding box of parry
		float parry_time_window = apply_bonus(PARRY_TIME_WINDOW, player->special_duration);
		float radius = size.x;
		Entity parry_entity = createParry(player_position.position, mouse_position, parry_time_window, 1, radius, size);
		player->special_timer = (SPECIAL_COOLDOWN - player->special_cooldown_reduction.flat_bonus) * (1 - player->special_cooldown_reduction.percent_bonus);
		break;
	}

	changeState(player_entity, PLAYER_ATTACKING);
}

void PlayerSystem::pickup_item(Entity& item_entity) {
	Item& item = registry.items.get(item_entity);
	Player& player = registry.players.components[0];
	Living& player_living = registry.livings.get(player_entity);

	// if (item.info.rarity != ItemRarity::SPECIAL) {
	// 	player.inventory.push_back(item.info);
	// }

	bool found = false;
	for (auto& inventory_item : player.inventory) {
		if (inventory_item.item_name == item.info.item_name) {
			inventory_item.quantity++;
			found = true;
			break;
		}
	}

	if (!found && item.info.rarity != ItemRarity::SPECIAL) {
		item.info.quantity = 1;
		player.inventory.push_back(item.info);
	}

	// TODO: Add bonuses + whatever else
	player.attack_bonus += item.info.attack_bonus;
	player.special_attack_bonus += item.info.special_attack_bonus;
	player.health_bonus += item.info.health_bonus;
	player.defense_bonus += item.info.defense_bonus;
	player.crit_rate_bonus += item.info.crit_rate_bonus;
	player.crit_damage_bonus += item.info.crit_damage_bonus;
	player.speed_bonus += item.info.speed_bonus;
	
	// update living stats
	update_stats(player, player_living);

	if (PICKUP_ITEM_ENUM_MAP[item.info.item_name] == PICKUP_ITEMS::SOULS) {

		assert(registry.gameStates.size() == 1);
		GameState& game_state = registry.gameStates.components[0];
		game_state.souls += item.value;
	}

	if (PICKUP_ITEM_ENUM_MAP[item.info.item_name] == PICKUP_ITEMS::HEALTH) {

		player_living.current_health += item.value / 100.0 * player_living.max_health;
	}

	// make sure it doesn't overheal
	if (player_living.current_health > player_living.max_health) {
		player_living.current_health = player_living.max_health;
	}

	Entity& healthbar_entity = registry.healthbar.entities[0];
	HealthBar& healthbar = registry.healthbar.get(healthbar_entity);
	healthbar.health = player_living.current_health;
}

// Every stat must be over 0
float PlayerSystem::apply_bonus(float value, Modifier bonus) {
	return max((value + bonus.flat_bonus) * (1.f + bonus.percent_bonus), 1.f);
}

// step parry
void PlayerSystem::step_parry(float elapsed_ms) {

	auto& parry_registry = registry.parries;

	// step parry timer w/ elapsed time, remove if above 
	for (Entity parry_entity : parry_registry.entities)
	{
		Parry& parry = parry_registry.get(parry_entity);
		parry.parry_time_window -= elapsed_ms;

		// make parry follow player
		Entity& player_entity = registry.players.entities[0];
		Velocity& player_velocity = registry.velocities.get(player_entity);
		
		registry.velocities.get(parry_entity) = player_velocity;

		if (parry.parry_time_window <= 0.0) {
			registry.remove_all_components_of(parry_entity);
		}
	}
}

void PlayerSystem::apply_upgrades(std::vector<UpgradeInfo> upgrades) {
	Living& player_living = registry.livings.get(player_entity);

	for (UpgradeInfo upgrade : upgrades) {
		player->attack_bonus += upgrade.attack_bonus;
		player->special_attack_bonus += upgrade.special_attack_bonus;
		player->health_bonus += upgrade.health_bonus;
		player->defense_bonus += upgrade.defense_bonus;
		player->crit_rate_bonus += upgrade.crit_rate_bonus;
		player->crit_damage_bonus += upgrade.crit_damage_bonus;
		player->speed_bonus += upgrade.speed_bonus;

		player->special_cooldown_reduction += upgrade.special_cooldown_reduction;
		player->special_duration += upgrade.special_duration;

		// update living stats
		update_stats(*player, player_living);
	}
}