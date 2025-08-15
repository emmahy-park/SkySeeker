#pragma once
#include "common.hpp"
#include <vector>
//#include <unordered_map>
#include "../ext/stb_image/stb_image.h"
#include "../data/data_structs.hpp"
#include "../data/states.hpp"
#include "../data/enemy_types.hpp"


// Player component
struct Player
{
	float base_speed = 0.0f;
	float current_speed = 0.0f;

	float base_max_health = 100.0f;
	float base_attack = 35.0f;
	float base_defense = 10.0f;
	float base_special_attack = 30.0f;
	float base_crit_rate = .05f;
	float base_crit_damage = 2.0f;

	Modifier health_bonus = { 0, 0 };
	Modifier attack_bonus = { 0, 0 };
	Modifier defense_bonus = { 0, 0 };
	Modifier special_attack_bonus = { 0, 0 };
	float crit_rate_bonus = 0.f;
	float crit_damage_bonus = 0.f;

	Modifier speed_bonus = { 0, 0 };

	Modifier special_cooldown_reduction = { 0.0f, 0.0f };
	Modifier special_duration = { 0.0f, 0.0f };

	PLAYER_STATES current_state = PLAYER_IDLE;
	SPECIAL_STATES current_special = SPECIAL_PARRY;
	float dash_timer = 0.0f;
	float sword_timer = 0.0f;
	float range_timer = 0.0f;
	float charge_time = 0.f;
	float special_timer = 0.0f;

	std::vector<ItemInfo> inventory; // Used if for displaying and recalculating stats for whatever reason.
};

struct Living {
	float max_health = 125.0f;
	float current_health = 125.0f;
	float attack = 35.0f;
	float defense = 10.0f;
	float special_attack = 15.0f;
	float crit_rate = .05f;
	float crit_damage = 2.0f;
	float invincible_time = 0.0f;
};

// Tower
struct Tower {
	float range;	// for vision / detection
	int timer_ms;	// when to shoot - this could also be a separate timer component...
};

// Invader
struct Enemy {
	ENEMY_TYPE type = CHARGER;
	bool sees_player = false;
	float speed = 0.0f;
	float flinch_timer_ms = 0.0f;
	ENEMY_STATES current_state = ENEMY_IDLE;
	float windup_timer_ms = 0.f;
	float attack_timer_ms = 0.f;
	float recovery_timer_ms = 0.f;
	int souls_value = 0;
	int health_value = 0;
	float attack_range = 0.0f;
};

struct Item {
	ItemInfo info; // Information about this item

	float value; // Used for pickup items
	bool is_pickup; // Should item pickup on touch
	bool can_pickup = true;; // Can item currently be picked up

	float time_alive = 0.f;
};

struct Sword {
	float radius = 0.0; // distance from center of player to center of sword
	float angle = 0.0;
	float mouse_angle = 0.0;
	float start_angle = 0.0;
	float end_angle = 0.0;
	float swing_progress = 0.0; // [0.0, 1.0]
	float swing_speed = 0.0;
};

struct Attack {
	Entity attacker;
	vec2 target_position;
};

// Projectile
struct Projectile {
	float damage_multiplier;
	float range = 0.0;
	vec2 initial_position = { 0.0, 0.0 };
};

// Parry
struct Parry {
	float parry_time_window = 0.0; // window of time for parry
};

struct Collidable { // if scale is (0, 0) ignore values
	vec2 position = { 0, 0 }; // Relative to position component
	vec2 scale = { 0, 0 }; 
};

// [2] Used for 2D transformation
struct Position {
	vec2 position = { 0, 0 };
	float layer = 0;
	vec2 scale = { 10, 10 }; // For collision
	float angle = 0; // In degrees
};

struct Velocity {
	vec2 velocity = { 0, 0 };
};

struct MoveFunction {
	vec2 (*velocity_function)(float); // Changes velocity based on function   
	float max_time = 0.0f;
	float current_time = 0.0f;
};

// Stores movement instruction
struct MoveNode {
	vec2 position = { 0.f, 0.f };
};


struct PathNode {
	vec2 position = { 0.f, 0.f };
	std::vector<PathNode*> edges;
	int id;
	int node_to_connect;
};

struct MapTile {
	bool exit = false;
};

struct Entrance {

};

struct EnemySpawn {

};

struct ItemSpawn {

};

struct LightSource {
	float radius = 500.f;
};

// [1] TextureInfo component
struct TextureInfo {
	vec2 top_left = { 0, 0 };
	vec2 bottom_right = { 1, 1 };
	GLuint texture_id = 0;
};

// Creative Component: Particle System
enum PARTICLE_TYPE {
	LINEAR = 0,
	ACCELERATED
};

struct Particle {
	PARTICLE_TYPE type = LINEAR;
	float life_time;
	vec2 velocity = { 0, 0 };
	vec2 acceleration = { 0, 0 };
};

enum ANIMATION_TYPE {
	LINEAR_ANIMATION = 0,
	STEP_ANIMATION,
	SINGLE_LINEAR_ANIMATION
};

struct Animation {
	int current_frame = 0;
	int max_frames = 1;
	ANIMATION_TYPE type = LINEAR_ANIMATION;
	AnimationInfo info; // see data_structs.hpp
};

// Stucture to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other; // the second object involved in the collision
	Collision(Entity& other) { this->other = other; };
};

// Data structure for toggling debug mode
struct Debug {
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
};
extern Debug debugging;

// Sets the brightness of the screen
struct ScreenState
{
	vec2 camera_position = vec2(WINDOW_WIDTH_PX / 2, WINDOW_WIDTH_PX / 2);
	float darken_screen_factor = -1;
	bool is_paused = false;
	bool is_death = false;
	bool tutorialActive = false;
	int tutorialStep;
	bool isInventoryClosed = true;
	vec2 item_position = {0, 0};
};

struct GameState {
	float souls;
	std::vector<std::string> unlocked_upgrades;
};

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
};

// used to hold grid line start and end positions
struct GridLine {
	vec2 start_pos = {  0,  0 };
	vec2 end_pos   = { 10, 10 };	// default to diagonal line
};

// A timer that will be associated to dying chicken
struct DeathTimer
{
	float counter_ms = 3000;
};

// Triangle for triangle collision
struct Triangle
{
	vec2 vertices[3]; // array of (x, y) coordinates

	Triangle(vec2 v0, vec2 v1, vec2 v2) {
		vertices[0] = v0;
		vertices[1] = v1;
		vertices[2] = v2;
	}
};

struct TriangleMesh
{
	std::vector<Triangle> triangle_mesh;
};

// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl & chicken.vs.glsl)
struct ColoredVertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
	vec2 original_size = {1,1};
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

// Fonts
struct Font
{
	std::string text;
	vec2 position;
	vec3 color;
	float scale;
	float maxWidth;
	std::string fontName = "KenneyPixel";
};

// Health Bar
struct HealthBar
{
	float health;
};

/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID {
	INVADER = 0,
	TOWER = INVADER + 1,
	PROJECTILE = TOWER + 1,
	HEALTH_BAR = PROJECTILE + 1,
	LIGHT_MAP = HEALTH_BAR + 1,
	TEXTURE_COUNT
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
	EGG = COLOURED + 1,
	CHICKEN = EGG + 1,
	TEXTURED = CHICKEN + 1,
	VIGNETTE = TEXTURED + 1,
	FONT = VIGNETTE + 1,
	PARTICLE = FONT + 1,
	SHADOW_MAP = PARTICLE + 1,
	SHADOW = SHADOW_MAP + 1,
	EFFECT_COUNT
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
	CHICKEN = 0,
	SPRITE = CHICKEN + 1,
	EGG = SPRITE + 1,
	DEBUG_LINE = EGG + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	SHADOW = SCREEN_TRIANGLE + 1,
	GEOMETRY_COUNT
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

enum class PARTICLE_TEXTURE_ID {
	TEST_PARTICLE = 0,
	BUBBLE,
	PARTICLE_COUNT
};
const int particle_count = (int)PARTICLE_TEXTURE_ID::PARTICLE_COUNT;