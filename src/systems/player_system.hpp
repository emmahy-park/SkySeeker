#pragma once

#include "../data/states.hpp"
#include "../data/data_structs.hpp"
#include "render_system.hpp"
#include "../common.hpp"
#include "../tinyECS/registry.hpp"

class PlayerSystem {
	void changeState(Entity e, PLAYER_STATES);
	void updateMovement(Entity player_entity);
	void updateTexture(Entity player_entity);
	void handle_dash();

	bool can_ranged_attack();
	bool can_melee_attack();
	bool can_special();

	Entity player_entity = Entity::get_null_entity();
	Player* player;

	void update_base_values(Player& player, Living& living);
	void update_stats(Player& player, Living& living);

	float apply_bonus(float value, Modifier bonus);

	const std::string SPRITE_PATH = textures_path("player/placeholder_player_sprites.png");
	const std::string SWORD_PATH = textures_path("attacks/slash.png");
	const std::string PROJECTILE_PATH = textures_path("projectiles/white_bubble.png");

	GLuint player_handle;
	GLuint sword_handle;
	GLuint projectile_handle;
	std::array<std::vector<TextureCoords>, num_animations> texture_coords; // Dynamic because there is a dynamic number of animations. Could be hard-coded.

	enum DIRECTIONS {
		NORTH = 0,
		EAST,
		SOUTH,
		WEST
	};

	bool flipped = false;

	bool override_texture = false;
	PLAYER_STATES override_texture_state = PLAYER_STATE_COUNT;

	const vec2 ACTUAL_SIZE = vec2(22, 27);
	const vec2 COLLISION_OFFSET = vec2(0, 0);

	const float RELATIVE_SIZE = 0.75f * BASE_TILE_SIZE_HEIGHT / ACTUAL_SIZE.y;
	const vec2 SPRITE_SIZE = vec2(64, 64) * RELATIVE_SIZE;
	const vec2 COLLISION_SIZE = ACTUAL_SIZE * RELATIVE_SIZE;

	const float BASE_SPEED = BASE_TILE_SIZE_HEIGHT * 3.f;

	std::array<bool, 4> movement_pressed = { false, false, false, false };

	const float BASE_RANGED_ATTACK_SPEED = 500.0f;
	const vec2 BASE_RANGED_ATTACK_SIZE = vec2(25.f, 25.f);
	const float BASE_RANGED_ATTACK_RANGE = 500.0f;
	const float BASE_RANGED_ATTACK_PERIOD = 1000.f;

	float MAX_CHARGE_TIME = 1000.f;

public:
	void init(RenderSystem& renderer);

	void step(float elapsed_ms);

	void on_key(int key, int action, int mod);

	Entity create_player(RenderSystem& renderer, std::vector<UpgradeInfo> current_upgrades);

	bool is_moving();

	Entity createSwordSwing(vec2 sword_owner_position, vec2 mouse_position, float swing_angle, float swing_speed, float layer, float radius, vec2 size);
	void swing_swords(float elapsed_ms, Entity& player_entity);

	Entity createParry(vec2 parry_owner_position, vec2 mouse_position, float parry_time_window, float layer, float radius, vec2 size);

	bool do_ranged_attack(vec2 mouse_position);
	void do_melee_attack(vec2 mouse_position);
	void do_special(vec2 mouse_position);

	void start_charging();

	void pickup_item(Entity& item_entity);

	void step_parry(float elapsed_ms);
	void apply_upgrades(std::vector<UpgradeInfo> upgrades);
};