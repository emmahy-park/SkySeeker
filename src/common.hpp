#pragma once

// stlib
#include <fstream> // stdout, stderr..
#include <string>
#include <tuple>
#include <vector>

// glfw (OpenGL)
#define NOMINMAX
#include <gl3w.h>
#include <GLFW/glfw3.h>

// The glm library provides vector and matrix operations as in GLSL
#include <glm/vec2.hpp>				// vec2
#include <glm/ext/vector_int2.hpp>  // ivec2
#include <glm/vec3.hpp>             // vec3
#include <glm/mat3x3.hpp>           // mat3
using namespace glm;

#include "tinyECS/tiny_ecs.hpp"

// Simple utility functions to avoid mistyping directory name
// audio_path("audio.ogg") -> data/audio/audio.ogg
// Get defintion of PROJECT_SOURCE_DIR from:
#include "../ext/project_path.hpp"
inline std::string data_path() { return std::string(PROJECT_SOURCE_DIR) + "data"; };
inline std::string font_path(const std::string& name) { return data_path() + "/fonts/" + name; };
inline std::string shader_path(const std::string& name) {return std::string(PROJECT_SOURCE_DIR) + "/shaders/" + name;};
inline std::string textures_path(const std::string& name) {return data_path() + "/textures/" + std::string(name);};
inline std::string audio_path(const std::string& name) {return data_path() + "/audio/" + std::string(name);};
inline std::string mesh_path(const std::string& name) {return data_path() + "/meshes/" + std::string(name);};
inline std::string json_path(const std::string& name) { return data_path() + "/json/" + std::string(name); };

//
// game constants
//
const int WINDOW_WIDTH_PX = 980;
const int WINDOW_HEIGHT_PX = 720;

// const int BACKGROUND_COLOUR = 0x0a1f42;
// 5b72a4
const vec3 BACKGROUND_COLOUR = vec3(0x17, 0x28, 0x32) / 255.f;

const int AI_PATHFINDING_INTERVAL_MS = 200;	// number of milliseconds between pathfinding passes

const int GRID_CELL_WIDTH_PX = 60;
const int GRID_CELL_HEIGHT_PX = 60;
const int GRID_LINE_WIDTH_PX = 2;

const int TOWER_TIMER_MS = 1000;	// number of milliseconds between tower shots
const int MAX_TOWERS_START = 5;

const int INVADER_HEALTH = 50;

const float HEALTH_DROP_HEAL_PERCENT = 20.0f;
const float HEALTH_DROP_SPAWN_PERCENT = 25.0f;

const int PROJECTILE_DAMAGE = 10;

const int BASE_TILE_SIZE_WIDTH = 75;
const int BASE_TILE_SIZE_HEIGHT = 75;

const vec2 ITEM_SIZE = vec2(BASE_TILE_SIZE_WIDTH, BASE_TILE_SIZE_HEIGHT) * 0.75f;

const float BASE_SWING_SPEED = 5.0f;

const float ITEM_PICKUP_RANGE = BASE_TILE_SIZE_WIDTH * 1.5f;
const float ITEM_PICKUP_ACCELERATION = BASE_TILE_SIZE_WIDTH * 0.5f;
const float ITEM_PICKUP_SPREAD_TIME = 500.f;
const float ITEM_PICKUP_MAX_SPREAD_DISTANCE = BASE_TILE_SIZE_WIDTH;
const float ITEM_PICKUP_MIN_DELAY = 250.f;

const float BASE_DASH_TIME = 150.f;

const float DASH_SPEED_MODIFIER = 4.0f;

const float BASE_DASH_COOLDOWN = 2000.f; // 2 seconds

const float PROJECTILE_LAYER = 1.f;

const int CITY_MAPS = 14;

const float HEALTHBAR_XPOS = -450.f;
const float HEALTHBAR_YPOS = -320.f;

const float SPECIAL_COOLDOWN = 2000.0f;
const float PARRY_TIME_WINDOW = 200.0f;
const float PARRY_KNOCKBACK_DIST = 100.0f;
const float PARRY_FLINCH_TIME = 2000.0f;

const float VISIBILITY_THRESHOLD = 50.0f;

const int BOSS_ROOM_LEVEL = 15;

// These are hard coded to the dimensions of the entity's texture

// invaders are 64x64 px, but cells are 60x60
const float INVADER_BB_WIDTH = (float)GRID_CELL_WIDTH_PX;
const float INVADER_BB_HEIGHT = (float)GRID_CELL_HEIGHT_PX;

// towers are 64x64 px, but cells are 60x60
const float TOWER_BB_WIDTH = (float)GRID_CELL_WIDTH_PX;
const float TOWER_BB_HEIGHT = (float)GRID_CELL_HEIGHT_PX;

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#ifndef ONE_OVER_ROOT_TWO
#define ONE_OVER_ROOT_TWO 0.70710678118f // Hard-coded for speed maybe
#endif

#ifndef ROOT_TWO
#define ROOT_TWO 1.414213562373f // Hard-coded for speed maybe
#endif

// The 'Transform' component handles transformations passed to the Vertex shader
// (similar to the gl Immediate mode equivalent, e.g., glTranslate()...)
// We recommend making all components non-copyable by derving from ComponentNonCopyable
struct Transform {
	mat3 mat = { { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f}, { 0.f, 0.f, 1.f} }; // start with the identity
	void scale(vec2 scale);
	void rotate(float radians);
	void translate(vec2 offset);
};

bool gl_has_errors();
