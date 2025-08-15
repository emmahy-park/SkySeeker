#include "enemy_system.hpp"
#include "world_init.hpp"
#include "../config.hpp"
#include <iostream>

// Initializes enemy sprites and texture info
// based on code written in player_system.cpp
void EnemySystem::init(RenderSystem& renderer) {
	// Load textures
	//renderer.loadGlTextures(&enemy_handle, &SPRITE_PATH, 1);

	renderer.loadGlTextures(&projectile_handle, &PROJECTILE_PATH, 1);

	for (int i = 0; i < enemy_type_count; i++) {
		Enemy_Template& enemyTemplate = enemy_templates.at((ENEMY_TYPE)i);
		renderer.loadGlTextures(&enemyTemplate.enemy_handle, &enemyTemplate.SPRITE_PATH, 1);

		float vertical_size = 1.0f / enemy_animation_states;
		float horizontal_size = 1.0f / MAX_ENEMY_ANIMATIONS;

		for (int i = 0; i < enemy_state_count; i++) {
			enemyTemplate.texture_coords[i] = std::vector<TextureCoords>(num_enemy_animations[i], TextureCoords());
			for (int j = 0; j < num_enemy_animations[i]; j++) {
				TextureCoords& tc = enemyTemplate.texture_coords[i][j];
				tc.top_left = vec2(j * horizontal_size, i * vertical_size);
				tc.bottom_right = vec2((j + 1) * horizontal_size, (i + 1) * vertical_size);
			}
		}
	}

	rng = std::default_random_engine(std::random_device()());
}

void EnemySystem::changeState(Entity e, ENEMY_STATES state) {
	Enemy& enemy_component = registry.enemies.get(e);
	Animation& a = registry.animations.get(e);
	const Enemy_Template& enemyTemplate = enemy_templates.at(enemy_component.type);

	assert(state < ENEMY_STATE_COUNT);

	enemy_component.current_state = state;
	a.max_frames = num_enemy_animations[(int)state];
	a.current_frame = 0; // reset frames

	// Setup animation
	switch (state) {
	case ENEMY_FLINCH:
		if (registry.attacks.has(e)) {
			registry.attacks.remove(e);
		}
		enemy_component.flinch_timer_ms = 50.f;
		a.type = LINEAR_ANIMATION;
		a.info.la.animation_step = 200.0f; // 200 ms
		a.info.la.animation_ms = 0.0f;
		break;
	case ENEMY_RECOVERING:
		//enemy_component.recovery_timer_ms = enemyTemplate.recovery_time_ms;
		//std::cerr << "enemy recovering";
		break;
	case ENEMY_ATTACKING:
		//enemy_component.attack_timer_ms = enemyTemplate.attack_time_ms;
		//std::cerr << "enemy attacking";
		break;
	case ENEMY_WINDING_UP:
		//std::cerr << "enemy winding up";
		break;
	case ENEMY_WALKING:
		//std::cerr << "enemy walking";
	case ENEMY_IDLE:
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

// this function should be called when an enemy is damaged
void EnemySystem::react_to_damage(Entity& entity) {
	changeState(entity, ENEMY_FLINCH);
}

void EnemySystem::react_to_parry(Entity& entity) {
	changeState(entity, ENEMY_FLINCH);
	vec2& enemy_velocity = registry.velocities.get(entity).velocity;

	Entity player_entity = registry.players.entities[0];
	vec2 player_position = registry.positions.get(player_entity).position;
	vec2 enemy_position = registry.positions.get(entity).position;
	vec2 knockback_direction = normalize(enemy_position - player_position);

	enemy_velocity = knockback_direction * PARRY_KNOCKBACK_DIST;

	Enemy& enemy_component = registry.enemies.get(entity);

	enemy_component.flinch_timer_ms = PARRY_FLINCH_TIME;
}

void EnemySystem::updateTexture(Entity enemy_entity) {
	if (!registry.positions.has(enemy_entity)) {
		return;
	}
	Position& pos = registry.positions.get(enemy_entity);
	Enemy& enemy = registry.enemies.get(enemy_entity);
	// Set texture info
	TextureInfo& ti = registry.textureinfos.get(enemy_entity);
	Animation& a = registry.animations.get(enemy_entity);
	const Enemy_Template& enemyTemplate = enemy_templates.at(enemy.type);

	pos.scale = flipped ? vec2(-enemyTemplate.SPRITE_SIZE.x, enemyTemplate.SPRITE_SIZE.y) : enemyTemplate.SPRITE_SIZE;
	pos.scale = pos.scale * enemyTemplate.RELATIVE_SIZE;

	const TextureCoords& tc = enemyTemplate.texture_coords[enemy.current_state][a.current_frame];

	ti.top_left = tc.top_left;
	ti.bottom_right = tc.bottom_right;
}

Velocity& setVelocity(Velocity& velocity, vec2& start, vec2& end, float speed) {
	float distX = end[0] - start[0];
	float distY = end[1] - start[1];
	float dist = sqrtf(distX * distX + distY * distY);
	velocity.velocity[0] = speed * (distX / dist);
	velocity.velocity[1] = speed * (distY / dist);
	return velocity;
}


// for all entities with a moveNode, sets their velocity
// vector using the angle of the vector from current position
// to moveNode position and the enemy's speed attribute
// No longer assumes a movenode has an enemy
void EnemySystem::updateMovement() {
	for (Entity& entity : registry.moveNodes.entities) {
		//assert(registry.enemies.has(entity) && "MoveNode without an Enemy!");
		if (!registry.enemies.has(entity)) continue;

		ComponentContainer<Velocity>& velocities_registry = registry.velocities;
		if (!velocities_registry.has(entity)) {
			velocities_registry.emplace(entity);
		}

		Velocity& velocity = velocities_registry.get(entity);
		if (registry.enemies.get(entity).current_state == ENEMY_FLINCH) {
			velocity.velocity[0] = 0;
			velocity.velocity[1] = 0;
			continue;
		}

		Enemy& enemy = registry.enemies.get(entity);
		float speed = enemy.speed;
		if (speed == 0) {
			if (velocities_registry.has(entity)) {
				velocity.velocity[0] = 0;
				velocity.velocity[1] = 0;
			}
			continue;
		}

		//if (enemy.current_state == ENEMY_WINDING_UP || enemy.current_state == ENEMY_ATTACKING || enemy.current_state == ENEMY_RECOVERING) continue;

		MoveNode& moveNode = registry.moveNodes.get(entity);
		Position& position = registry.positions.get(entity);
		setVelocity(velocity, position.position, moveNode.position, speed);
		if (enemy.current_state == ENEMY_IDLE) {
			changeState(entity, ENEMY_WALKING);
		}
	}
}

// handles behavior for an enemy to prepare an attack and enter their "winding up" state
void EnemySystem::prepare_attack(Entity& attacker, vec2& position) {
	Enemy& enemy = registry.enemies.get(attacker);
	const Enemy_Template& enemyTemplate = enemy_templates.at(enemy.type);
	enemy.windup_timer_ms = enemyTemplate.windup_time_ms;
	enemy.attack_timer_ms = enemyTemplate.attack_time_ms;
	enemy.recovery_timer_ms = enemyTemplate.recovery_time_ms;
	changeState(attacker, ENEMY_WINDING_UP);
	if (enemy.type == CHARGER || enemy.type == RANGED || enemy.type == BOSS) {
		registry.moveNodes.remove(attacker);
		registry.velocities.get(attacker).velocity = { 0, 0 };
	}
	Attack& attack_component = registry.attacks.emplace(attacker);
	attack_component.attacker = attacker;
	attack_component.target_position.x = position.x;
	attack_component.target_position.y = position.y;
	
}

// handles behavior for an enemy to execute their attack and enter their "attacking" state
void EnemySystem::execute_attack(Entity& attacker) {
	ComponentContainer<Attack>& attack_registry = registry.attacks;
	ComponentContainer<Enemy>& enemy_registry = registry.enemies;
	ComponentContainer<Position>& position_registry = registry.positions;
	ComponentContainer<Velocity>& velocity_registry = registry.velocities;


	Attack& attack_component = attack_registry.get(attacker);
		
	// if the attacking enemy is in attack state, give the attack a collision box and/or update
	// the attack's position/size
		
	Enemy& enemy_component = enemy_registry.get(attacker);
	Position& attacker_position = position_registry.get(attacker);
	Velocity& attacker_velocity = velocity_registry.get(attacker);
	float which_attack = 0;
	switch (enemy_component.type) {
	case CHARGER:
		chargeAttack(attack_component);
		break;
	case RANGED:
		projectileAttack(attack_component, 1);
		break;
	case BOSS:
		which_attack = rand()%2;
		if (which_attack > 0) {
			projectileAttack(attack_component, 5);
		}
		else {
			chargeAttack(attack_component);
		}
		break;
	default:
		break;
	}
}

void EnemySystem::chargeAttack(Attack& attack) {
	Position& attacker_position = registry.positions.get(attack.attacker);
	Velocity& attacker_velocity = registry.velocities.get(attack.attacker);
	attacker_velocity = setVelocity(attacker_velocity, attacker_position.position, attack.target_position, CHARGE_SPEED);
}

void EnemySystem::projectileAttack(Attack& attack, int num_projectiles) {
	Position& attacker_position = registry.positions.get(attack.attacker);
	Velocity& attacker_velocity = registry.velocities.get(attack.attacker);
	if (num_projectiles == 1) {
		createProjectile(attack.attacker, attacker_position.position, attack.target_position, BASE_TILE_SIZE_HEIGHT * 8.0f,
			BASE_TILE_SIZE_HEIGHT * 2.0f, vec2(25.f, 25.f), 2, projectile_handle);
	}
	else {
		float direction = atan2(attack.target_position.y,attack.target_position.x);
		//std::cerr << direction << std::endl;
		for (double i = 0 - (double)num_projectiles / 2; i < (double)num_projectiles / 2; i += 1) {
			vec2 target_pos = attack.target_position;
			target_pos.y = target_pos.x * tan(direction + (i * 0.1));
			createProjectile(attack.attacker, attacker_position.position, target_pos, BASE_TILE_SIZE_HEIGHT * 8.0f,
				BASE_TILE_SIZE_HEIGHT * 2.0f, vec2(25.f, 25.f), 2, projectile_handle);
		}
	}
	registry.attacks.remove(attack.attacker);
}


// updates flinch timer for enemy damage reaction
// updates attack timers for enemy attack behavior
void EnemySystem::updateTimers(float elapsed_ms) {
	for (Entity& enemy_entity : registry.enemies.entities) {
		Enemy& enemy = registry.enemies.get(enemy_entity);
		if (enemy.flinch_timer_ms > 0) {
			enemy.flinch_timer_ms -= elapsed_ms;
		}
		else if (enemy.flinch_timer_ms <= 0 && enemy.current_state == ENEMY_FLINCH) {
			changeState(enemy_entity, ENEMY_IDLE);
		}
		// windup
		else if (enemy.windup_timer_ms > 0) {
			enemy.windup_timer_ms -= elapsed_ms;
		}
		else if (enemy.windup_timer_ms <= 0 && enemy.current_state == ENEMY_WINDING_UP) {
			//std::cout << "Attack executing";
			changeState(enemy_entity, ENEMY_ATTACKING);
			// Create attack
			execute_attack(enemy_entity);
		}
		// attack
		else if (enemy.attack_timer_ms > 0) {
			enemy.attack_timer_ms -= elapsed_ms;
		}
		else if (enemy.attack_timer_ms <= 0 && enemy.current_state == ENEMY_ATTACKING) {
			changeState(enemy_entity, ENEMY_RECOVERING);

			// remove any attack components with this enemy
			registry.attacks.remove(enemy_entity);
			registry.velocities.get(enemy_entity).velocity = vec2(0, 0);
		}
		// recover
		else if (enemy.recovery_timer_ms > 0) {
			enemy.recovery_timer_ms -= elapsed_ms;
		}
		else if (enemy.recovery_timer_ms <= 0 && enemy.current_state == ENEMY_RECOVERING) {
			changeState(enemy_entity, ENEMY_IDLE);
			
		}
	}
}

void EnemySystem::step(float elapsed_ms) {
	enemy_cleanup_check();
	updateTimers(elapsed_ms);
	updateMovement();
	for (Entity& enemy_entity : registry.enemies.entities) {
		updateTexture(enemy_entity);
	}
}

bool EnemySystem::is_moving(Entity& entity) {
	return (registry.velocities.has(entity) 
		&& registry.velocities.get(entity).velocity.x != 0
		&& registry.velocities.get(entity).velocity.y != 0);
}

void EnemySystem::handle_enemy_death(Entity& enemy_entity) {
	if (registry.pathNodes.has(enemy_entity)) {
		// pathNodes are instantiated with heap memory
		delete registry.pathNodes.get(enemy_entity);
	}
	//registry.remove_all_components_of(enemy_entity);
	registry.enemies.remove(enemy_entity);
	registry.positions.remove(enemy_entity);
	registry.collidables.remove(enemy_entity);
	registry.velocities.remove(enemy_entity);
}

void EnemySystem::enemy_cleanup_check() {
	for (Entity& entity : registry.livings.entities) {
		if (!registry.positions.has(entity)) {
			for (Attack& attack : registry.attacks.components) {
				if (attack.attacker == entity) {
					return;
				}
			}
			registry.remove_all_components_of(entity);
		}
	}
}

Entity EnemySystem::create_enemy(RenderSystem& renderer, vec2 position, ENEMY_TYPE type) {
	const Entity& enemy_entity = Entity();
	const Enemy_Template& enemyTemplate = enemy_templates.at(type);

	auto& pos = registry.positions.emplace(enemy_entity);
	pos.position = position;
	pos.scale = enemyTemplate.SPRITE_SIZE * enemyTemplate.RELATIVE_SIZE;
	pos.layer = 1.0f;

	auto& collidable = registry.collidables.emplace(enemy_entity);
	collidable.position = enemyTemplate.COLLISION_OFFSET;
	collidable.scale = enemyTemplate.ACTUAL_SIZE * enemyTemplate.RELATIVE_SIZE;

	auto& velocity = registry.velocities.emplace(enemy_entity);
	velocity.velocity = vec2(0, 0);

	auto& enemy_component = registry.enemies.emplace(enemy_entity);
	enemy_component.speed = enemyTemplate.speed;
	enemy_component.attack_range = enemyTemplate.attack_range;
	enemy_component.souls_value = enemyTemplate.souls_value;
	enemy_component.health_value = enemyTemplate.health_value;
	enemy_component.type = type;

	auto& texture_info = registry.textureinfos.emplace(enemy_entity);
	texture_info.texture_id = enemyTemplate.enemy_handle;

	auto& animation = registry.animations.emplace(enemy_entity);

	auto& living = registry.livings.emplace(enemy_entity);
	living.max_health = enemyTemplate.max_health;
	living.current_health = enemyTemplate.current_health;
	living.attack = enemyTemplate.attack;
	living.defense = enemyTemplate.defense;
	living.special_attack = enemyTemplate.special_attack;
	living.crit_rate = enemyTemplate.crit_rate;
	living.crit_damage = enemyTemplate.crit_damage;
	living.invincible_time = enemyTemplate.invincible_time;

	changeState(enemy_entity, ENEMY_IDLE);

	return enemy_entity;
}