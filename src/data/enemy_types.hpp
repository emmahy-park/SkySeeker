#pragma once

#include "../common.hpp"
#include "data_structs.hpp"
#include "states.hpp"
#include <map>

enum ENEMY_TYPE {
	DEBUG_ENEMY = 0,
	CHARGER,
	RANGED,
	BOSS,
	NUM_ENEMY_TYPES
};
const int enemy_type_count = (int)NUM_ENEMY_TYPES;

struct Enemy_Template {
	float speed = BASE_TILE_SIZE_HEIGHT * 1.75f;
	float windup_time_ms = 0.f;
	float attack_time_ms = 0.f;
	float recovery_time_ms = 0.f;
	int souls_value = 5;
	int health_value = 1;
	float attack_range = 0.0f;

	const std::string SPRITE_PATH;

	GLuint enemy_handle;
	std::array<std::vector<TextureCoords>, enemy_state_count> texture_coords; // Dynamic because there is a dynamic number of animations. Could be hard-coded.

	bool flipped = false;
	vec2 ACTUAL_SIZE = vec2(42, 40);
	vec2 COLLISION_OFFSET = vec2(0, 0);

	float RELATIVE_SIZE = 0.5f * BASE_TILE_SIZE_HEIGHT / ACTUAL_SIZE.y;
	vec2 SPRITE_SIZE = vec2(48, 48) * RELATIVE_SIZE;

	float max_health = 100.0f;
	float current_health = 100.0f;
	float attack = 10.0f;
	float defense = 10.0f;
	float special_attack = 10.0f;
	float crit_rate = .05f;
	float crit_damage = 2.0f;
	float invincible_time = 0.0f;
};

static Enemy_Template debug_template = {
	0,
	0,
	0,
	0,
	0,
	0,
	0,

	textures_path("invaders/floater_3.png"),

	0,
	std::array<std::vector<TextureCoords>, enemy_state_count>(), // Dynamic because there is a dynamic number of animations. Could be hard-coded.

	false,
	vec2(42, 40),
	vec2(0, 0),

	0.5f * BASE_TILE_SIZE_HEIGHT / 40,
	vec2(48, 48) * (0.5f * BASE_TILE_SIZE_HEIGHT / 40),
	100.0f,
	100.0f,
	10.0f,
	10.0f,
	10.0f,
	.05f,
	2.0f,
	0.0f
};

static Enemy_Template charger_template = {
	BASE_TILE_SIZE_HEIGHT * 1.75f,	// speed
	500.f,							// windup time
	300.f,							// attack time
	1500.f,							// recovery time
	5,								// souls
	1,								// health drops
	BASE_TILE_SIZE_HEIGHT * 2.0f,	// range

	textures_path("enemies/Soldier.png"),

	0,	// texture handle
	std::array<std::vector<TextureCoords>, enemy_state_count>(), // Dynamic because there is a dynamic number of animations. Could be hard-coded.

	false,	// flipped?
	vec2(18, 22),	// actual size
	vec2(0, 0),		// collision offset

	0.85f * BASE_TILE_SIZE_HEIGHT / 22, // RELATIVE_SIZE
	vec2(100, 100), // SPRITE_SIZE
	100.0f,			// max_health
	100.0f,			// current_health
	25.0f,			// attack
	0.0f,			// defense
	0.0f,			// special_attack
	0.0f,			// crit_rate
	0.0f,			// crit_damage
	0.0f			// invincible time
};

static Enemy_Template ranged_template = {
	BASE_TILE_SIZE_HEIGHT * 1.5f,	// speed
	500.f,							// windup time
	150.f,							// attack time
	500.f,							// recovery time
	5,								// souls
	1,								// health drops
	BASE_TILE_SIZE_HEIGHT * 5.0f,	// range

	textures_path("enemies/golem.png"),

	0,	// texture handle
	std::array<std::vector<TextureCoords>, enemy_state_count>(), // Dynamic because there is a dynamic number of animations. Could be hard-coded.

	false,	// flipped?
	vec2(25, 25),	// actual size
	vec2(0, 0),		// collision offset

	0.75f * (BASE_TILE_SIZE_HEIGHT / 25), // RELATIVE_SIZE
	vec2(64, 64), // SPRITE_SIZE
	70.0f,			// max_health
	70.0f,			// current_health
	5.0f,			// attack
	0.0f,			// defense
	20.0f,			// special_attack
	0.0f,			// crit_rate
	0.0f,			// crit_damage
	0.0f			// invincible time
};

static Enemy_Template boss_template = {
	BASE_TILE_SIZE_HEIGHT * 0.5f,	// speed
	500.f,							// windup time
	150.f,							// attack time
	500.f,							// recovery time
	5,								// souls
	5,							// health drops
	BASE_TILE_SIZE_HEIGHT * 5.0f,	// range

	textures_path("enemies/golem.png"),

	0,	// texture handle
	std::array<std::vector<TextureCoords>, enemy_state_count>(), // Dynamic because there is a dynamic number of animations. Could be hard-coded.

	false,	// flipped?
	vec2(25, 25),	// actual size
	vec2(0, 0),		// collision offset

	2.0f * (BASE_TILE_SIZE_HEIGHT / 25), // RELATIVE_SIZE
	vec2(64, 64),	// SPRITE_SIZE
	800.0f,			// max_health
	800.0f,			// current_health
	10.0f,			// attack
	5.0f,			// defense
	10.0f,			// special_attack
	.05f,			// crit_rate
	2.0f,			// crit_damage
	0.0f			// invincible time
};

static std::map<ENEMY_TYPE, Enemy_Template> enemy_templates = {
	{DEBUG_ENEMY, debug_template},
	{CHARGER, charger_template},
	{RANGED, ranged_template},
	{BOSS, boss_template}
};
