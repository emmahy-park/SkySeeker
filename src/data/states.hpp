#pragma once

#include <array>

enum PLAYER_STATES {
	PLAYER_IDLE = 0,
	PLAYER_WALKING,
	PLAYER_ATTACKING,
	PLAYER_RANGE_ATTACK,
	PLAYER_DASHING,
	PLAYER_FALLING,
	PLAYER_FLINCH,
	PLAYER_DIE,
	PLAYER_CHARGING,
	PLAYER_STATE_COUNT
};
const int player_state_count = (int)PLAYER_STATE_COUNT;
const int num_animations = 8;

enum ENEMY_STATES {
	ENEMY_IDLE = 0,
	ENEMY_WALKING,
	ENEMY_ATTACKING,
	ENEMY_RECOVERING,
	ENEMY_FLINCH,
	ENEMY_WINDING_UP,
	ENEMY_DYING,
	ENEMY_STATE_COUNT
};
const int enemy_state_count = (int)ENEMY_STATE_COUNT;

const std::array<int, num_animations> num_player_animations = {
	8, 8, 4, 3, 4, 4, 2, 14
};

const int MAX_PLAYER_ANIMATIONS = 14; // for texture info calculations

const int enemy_animation_states = 7;

const std::array<int, enemy_animation_states> num_enemy_animations = {
	1,1,1,1,1,1,1
};

const int MAX_ANIMATIONS = 14; // for texture info calculations

enum SPECIAL_STATES {
	SPECIAL_PARRY = 0,
	SPECIAL_COUNT
};
const int MAX_ENEMY_ANIMATIONS = 9;
