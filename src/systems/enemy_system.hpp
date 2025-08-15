#pragma once

#include <unordered_map>
#include <random>
#include <cmath>
#include "../data/states.hpp"
#include "../data/data_structs.hpp"
#include "render_system.hpp"
#include "../common.hpp"
#include "../tinyECS/registry.hpp"
#include "../data/enemy_types.hpp"

class EnemySystem {
	void changeState(Entity e, ENEMY_STATES);
	void updateMovement();
	void updateTimers(float elapsed_ms);
	void updateTexture(Entity player_entity);

	//const std::string SPRITE_PATH = textures_path("invaders/floater_3.png");
	GLuint projectile_handle;
	const std::string PROJECTILE_PATH = textures_path("projectiles/gold_bubble.png");
	//GLuint enemy_handle;
	//std::array<std::vector<TextureCoords>, player_state_count> texture_coords; // Dynamic because there is a dynamic number of animations. Could be hard-coded.

	bool flipped = false;

	const float CHARGE_SPEED = (BASE_TILE_SIZE_HEIGHT * 6.f);

	//Enemy_Template enemy_templates[enemy_type_count];

	// [1] Improved Gameplay: AI
	void execute_attack(Entity& attacker);

	void chargeAttack(Attack& attack);

	void projectileAttack(Attack& attack, int num_projectiles);

	void enemy_cleanup_check();

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1

public:
	void init(RenderSystem& renderer);

	void step(float elapsed_ms);

	Entity create_enemy(RenderSystem& renderer, vec2 position, ENEMY_TYPE type);

	bool is_moving(Entity& entity);

	void react_to_damage(Entity& entity);

	void react_to_parry(Entity& entity);

	// [1] Improved Gameplay: AI
	void prepare_attack(Entity& attacker, vec2& position);

	void handle_enemy_death(Entity& enemy_entity);
};