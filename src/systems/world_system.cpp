// Header
#include "world_system.hpp"
#include "world_init.hpp"
#include "../util/util.hpp"
#include "../config.hpp"
#include "../data/items.hpp"
#include "enemy_system.hpp"
#include "particle_system.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>

#include "physics_system.hpp"
#include "../util/map_parser.hpp"
#include "../../ext/nlohmann/json.hpp"

bool WorldSystem::is_itempopup_visible = false;

// create the world
WorldSystem::WorldSystem() :
	points(0),
	max_towers(MAX_TOWERS_START)
{
	// seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem() {
	// Destroy music components
	if (background_start_music != nullptr)
		Mix_FreeMusic(background_start_music);
	if (sword_swing_sound != nullptr)
		Mix_FreeChunk(sword_swing_sound);
	if (laser_sound != nullptr)
		Mix_FreeChunk(laser_sound);
	if (background_game_music != nullptr)
		Mix_FreeMusic(background_game_music);
	if (background_paused_music != nullptr)
		Mix_FreeMusic(background_paused_music);
	if (souls_pickup != nullptr)
		Mix_FreeChunk(souls_pickup);
	if (item_equip != nullptr)
		Mix_FreeChunk(item_equip);
	if (menu_change != nullptr)
		Mix_FreeChunk(menu_change);
	if (enter != nullptr)
		Mix_FreeChunk(enter);
	if (parry != nullptr)
		Mix_FreeChunk(parry);
	if (dash != nullptr)
		Mix_FreeChunk(dash);
	Mix_CloseAudio();

	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);
}

// Debugging
namespace {
	void glfw_err_cb(int error, const char *desc) {
		std::cerr << error << ": " << desc << std::endl;
	}
}

// call to close the window, wrapper around GLFW commands
void WorldSystem::close_window() {
	glfwSetWindowShouldClose(window, GLFW_TRUE);
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow* WorldSystem::create_window() {

	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit()) {
		std::cerr << "ERROR: Failed to initialize GLFW in world_system.cpp" << std::endl;
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	// CK: setting GLFW_SCALE_TO_MONITOR to true will rescale window but then you must handle different scalings
	// glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_TRUE);		// GLFW 3.3+
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_FALSE);		// GLFW 3.3+

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX, "SKYSEEKER", nullptr, nullptr);
	if (window == nullptr) {
		std::cerr << "ERROR: Failed to glfwCreateWindow in world_system.cpp" << std::endl;
		return nullptr;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	auto mouse_button_pressed_redirect = [](GLFWwindow* wnd, int _button, int _action, int _mods) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_button_pressed(_button, _action, _mods); };
	
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, mouse_button_pressed_redirect);

	return window;
}

bool WorldSystem::start_and_load_sounds() {
	
	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		return false;
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		return false;
	}

	// M3 Creative Component: Audio Integration
	background_start_music = Mix_LoadMUS(audio_path("start_music.wav").c_str());
	background_game_music = Mix_LoadMUS(audio_path("game_music.wav").c_str());
	background_paused_music = Mix_LoadMUS(audio_path("paused_music.wav").c_str());
	sword_swing_sound = Mix_LoadWAV(audio_path("sword_swing.wav").c_str());
	laser_sound = Mix_LoadWAV(audio_path("laser.wav").c_str());
	souls_pickup = Mix_LoadWAV(audio_path("souls-pickup.wav").c_str());
	item_equip = Mix_LoadWAV(audio_path("item-equip.wav").c_str());
	menu_change = Mix_LoadWAV(audio_path("menu_change.wav").c_str());
	enter = Mix_LoadWAV(audio_path("enter.wav").c_str());
	parry = Mix_LoadWAV(audio_path("parry.wav").c_str());
	dash = Mix_LoadWAV(audio_path("dash.wav").c_str());

	Mix_VolumeChunk(sword_swing_sound, 32);
	Mix_VolumeChunk(laser_sound, 64);
	Mix_VolumeChunk(enter, 32);

	if (background_start_music == nullptr || sword_swing_sound == nullptr || laser_sound == nullptr || background_game_music == nullptr || 
		background_paused_music == nullptr || souls_pickup == nullptr || item_equip == nullptr || menu_change == nullptr || enter == nullptr
		|| parry == nullptr || dash == nullptr) {
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n %s\n %s\n %s\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("start_music.wav").c_str(),
			audio_path("sword_swing.wav").c_str(),
			audio_path("laser.wav").c_str(),
			audio_path("game_music.wav").c_str(),
			audio_path("paused_music.wav").c_str(),
			audio_path("souls-pickup.wav").c_str(),
			audio_path("item-equip.wav").c_str(),
			audio_path("menu_change.wav").c_str(),
			audio_path("enter.wav").c_str(),
			audio_path("parry.wav").c_str(),
			audio_path("dash.wav").c_str());

		return false;
	}

	return true;
}

void WorldSystem::init(RenderSystem* renderer_arg, PlayerSystem* player_system, EnemySystem* enemy_system, ItemSystem* item_system, ScreenManager* screen_manager, UpgradeSystem* upgrade_system, ParticleSystem* particle_system, AISystem* ai_system) {

	this->renderer = renderer_arg;
	this->player_system = player_system;
	this->enemy_system = enemy_system;
	this->item_system = item_system;
	this->screen_manager = screen_manager;
	this->upgrade_system = upgrade_system;
	this->particle_system = particle_system;
	this->ai_system = ai_system;

	registry.gameStates.emplace(game_state_entity);

	srand((int) std::chrono::system_clock::now().time_since_epoch().count());

	// start playing background music indefinitely
	std::cout << "Starting music..." << std::endl;
	Mix_PlayMusic(background_start_music, -1);

	// Load game state
	load_game_state();

	// Set all states to default
    restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update, float actual_elapsed_ms) {

	GameState& game_state = registry.gameStates.components[0];

	// Updating window title with points
	std::stringstream title_ss;
	

	time_since_last_second += actual_elapsed_ms;
	frames_since_last_second++;

	if (time_since_last_second > 1000.f) {
		
		fps_counter = frames_since_last_second / time_since_last_second * 1000.f; // To account for extra time
		time_since_last_second = 0.f;
		frames_since_last_second = 0;
	}

	if (screen_manager->getCurrentScreen() != ScreenType::StartScreen || screen_manager->getCurrentScreen() != ScreenType::UpgradeScreen
		|| screen_manager->getCurrentScreen() != ScreenType::CreditsScreen) {
			title_ss << "FPS: " << fps_counter;
		glfwSetWindowTitle(window, title_ss.str().c_str());
	} else {
		glfwSetWindowTitle(window, "SKYSEEKER");
	}
	

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
	    registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Update mouse position according to player movement (tied to camera movement)
	Entity player = registry.players.entities[0];
	Velocity vel = registry.velocities.get(player);
	mouse_pos_x += vel.velocity.x * elapsed_ms_since_last_update / 1000;
	mouse_pos_y += vel.velocity.y * elapsed_ms_since_last_update / 1000;

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: game over fade out
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	assert(registry.screenStates.components.size() <= 1);
    ScreenState &screen = registry.screenStates.components[0];

	// Update camera position
	auto& player_entity = registry.players.entities[0];
	Position& player_position = registry.positions.get(player_entity);
	screen.camera_position = player_position.position;

	float distance = glm::length(player_position.position - screen.item_position);
	if (distance > VISIBILITY_THRESHOLD) {
		is_itempopup_visible = false;
	}

	auto& healthbar_entity = registry.healthbar.entities[0];
	updateHealthbarPosition(healthbar_entity, screen.camera_position);

	for (Living& living : registry.livings.components) {
		living.invincible_time -= elapsed_ms_since_last_update;
	}

	// spawn more enemies if 1 or fewer enemies on map
	Entity screen_state_entity = renderer->get_screen_state_entity();
	ScreenState& screen_state = registry.screenStates.get(screen_state_entity);

	if (registry.enemies.size() <=1 && num_waves > 0 && !screen_state.tutorialActive) {
		spawn_enemy_pack();
		num_waves--;
	}

	selected_item_entity = NULL;

	step_projectile(elapsed_ms_since_last_update);

	if (!is_player_alive() && !screen_state.tutorialActive) {
		screen_manager->setScreen(ScreenType::DeathScreen);
		screen_state.is_death = true;
	}

	return true;
}

// M3 Creative Component: Audio Integration
// change background music based on the current screen
void WorldSystem::change_background_music(ScreenManager& screen_manager) {
	ScreenType currentScreen = screen_manager.getCurrentScreen();
	Mix_Music* new_music = nullptr;

	if (currentScreen == ScreenType::PlayScreen || currentScreen == ScreenType::TutorialScreen) {
		new_music = background_game_music;
	}
	if (currentScreen == ScreenType::PausedScreen || currentScreen == ScreenType::TutorialPausedScreen) {
		new_music = background_paused_music;
	}
	if (currentScreen == ScreenType::StartScreen || currentScreen == ScreenType::CreditsScreen) {
		new_music = background_start_music;
	}

	// only change music if it's different from the currently playing one
	if (new_music != current_music) {
		Mix_HaltMusic();
		Mix_PlayMusic(new_music, -1);
		current_music = new_music;
	}
}

// M3 Creative Component: Audio Integration
// helper function to use in Screen Manager
void WorldSystem::play_sound_effect(std::string sound) {
	if (sound == "menu_change") {
		Mix_PlayChannel(-1, menu_change, 0);
	}
	else if (sound == "enter") {
		Mix_PlayChannel(-1, enter, 0);
	}
}

// deal with projectile  stuff
void WorldSystem::step_projectile(float elapsed_ms) {

	auto& projectile_registry = registry.projectiles;

	// remove projectile if out of range
	for (Entity projectile_entity : projectile_registry.entities)
	{
		Projectile& projectile = projectile_registry.get(projectile_entity);
		vec2 current_position = registry.positions.get(projectile_entity).position;

		// get distance between initial position and current position
		float distance_travelled = distance(projectile.initial_position, current_position);

		if (distance_travelled > projectile.range) {
			registry.remove_all_components_of(projectile_entity);
		}
	}
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {

	std::cout << "Restarting..." << std::endl;

	// Debugging for memory/component leaks
	registry.list_all_components();

	// Reset the game speed
	current_speed = 1.f;

	points = 0;
	max_towers = MAX_TOWERS_START;

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all bug, eagles, ... but that would be more cumbersome
	while (registry.positions.entities.size() > 0)
	    registry.remove_all_components_of(registry.positions.entities.back());

	// debugging for memory/component leaks
	registry.list_all_components();

	if (registry.players.entities.size() == 0) {
		std::vector<UpgradeInfo> upgrades;
		upgrade_system->get_current_upgrades(upgrades);
		player_system->create_player(*renderer, upgrades);
		createHealthBar(renderer);
	}

	// load map
	map_loader.parseMaps(textures_path("map/city_0.json"), renderer, ai_system);

	num_waves = 0;

	glfwSetWindowTitle(window, "SKYSEEKER");
}

// Handles collisions between entities and map obstacles/walls
// Tile Collision
void WorldSystem::handle_tile_collision(Entity& tile_entity, Entity& other_entity) {
	MapTile& tile = registry.mapTiles.get(tile_entity);
	Position& tile_pos = registry.positions.get(tile_entity);
	Collidable& tile_collidable = registry.collidables.get(tile_entity);

	if (registry.players.has(other_entity) || registry.enemies.has(other_entity)) { // Handle movement collision
		Position& pos = registry.positions.get(other_entity);
		Velocity& vel = registry.velocities.get(other_entity);
		Collidable& collidable = registry.collidables.get(other_entity);

		vec2 tile_size = get_bounding_box(tile_pos, tile_collidable);
		vec2 other_size = get_bounding_box(pos, collidable);
		// Get colliding side.

		// Get sides based on velocity.
		// Get the left side if velocity is negative (colliding from the right) or right if velocity is positive.
		float other_horizontal = vel.velocity.x < 0 ? pos.position.x - other_size.x / 2.0f : pos.position.x + other_size.x / 2.0f;
		float tile_horizontal = vel.velocity.x < 0 ? tile_pos.position.x + tile_size.x / 2.0f : tile_pos.position.x - tile_size.x / 2.0f;

		float other_vertical = vel.velocity.y < 0 ? pos.position.y - other_size.y / 2.0f : pos.position.y + other_size.y / 2.0f;
		float tile_vertical = vel.velocity.y < 0 ? tile_pos.position.y + tile_size.y / 2.0f : tile_pos.position.y - tile_size.y / 2.0f;

		// get times to collide. Does this by calculating backwards.
		float vert_time = std::numeric_limits<float>::infinity();
		if (vel.velocity.y != 0) vert_time = (tile_vertical - other_vertical) / (-vel.velocity.y);

		float horz_time = std::numeric_limits<float>::infinity();
		if (vel.velocity.x != 0) horz_time = (tile_horizontal - other_horizontal) / (-vel.velocity.x);

		if (vert_time < horz_time) {
			pos.position.y += vert_time * (-vel.velocity.y);
		}
		else if (horz_time < vert_time) { 
			pos.position.x += horz_time * (-vel.velocity.x);
		} // Ignore the corner
	}
}

// enemy-enemy collisions to prevent overlapping, currently unused due to bugs
void WorldSystem::handle_friendly_collision(Entity& entity0, Entity& entity1) {
	Position& pos0 = registry.positions.get(entity0);
	Velocity& vel0 = registry.velocities.get(entity0);
	Collidable& collidable0 = registry.collidables.get(entity0);

	Position& pos1 = registry.positions.get(entity1);
	Velocity& vel1 = registry.velocities.get(entity1);
	Collidable& collidable1 = registry.collidables.get(entity1);

	vec2 size0 = get_bounding_box(pos0, collidable0);
	vec2 size1 = get_bounding_box(pos1, collidable1);
	// Get colliding side.

	// Get sides based on velocity.
	// Get the left side if velocity is negative (colliding from the right) or right if velocity is positive.
	float horizontal0 = vel1.velocity.x < vel0.velocity.x ? pos0.position.x + size0.x / 2.0f : pos0.position.x - size0.x / 2.0f;
	float horizontal1 = vel1.velocity.x < vel0.velocity.x ? pos1.position.x - size1.x / 2.0f : pos1.position.x + size1.x / 2.0f;
	
	float vertical0 = vel1.velocity.y < vel0.velocity.y ? pos0.position.y + size0.y / 2.0f : pos0.position.y - size0.y / 2.0f;
	float vertical1 = vel1.velocity.y < vel0.velocity.y ? pos1.position.y - size1.y / 2.0f : pos1.position.y + size1.y / 2.0f;
	
	// get times to collide. Does this by calculating backwards.
	//float vert_time = 0;
	//if (vel1.velocity.y - vel0.velocity.y != 0) vert_time = (vertical0 - vertical1) / (-1*abs(vel0.velocity.y - vel1.velocity.y));

	//float horz_time = 0;
	//if (vel1.velocity.x - vel0.velocity.x != 0) horz_time = (horizontal0 - horizontal1) / (-1 * abs(vel0.velocity.x - vel1.velocity.x));
	float horiz_overlap = abs(horizontal0 - horizontal1);
	float vert_overlap = abs(vertical0 - vertical1);

	if (vel1.velocity.y < vel0.velocity.y) {
		pos0.position.y -= vert_overlap / 2.f;
		pos1.position.y += vert_overlap / 2.f;
	}
	else if (vel1.velocity.y > vel0.velocity.y) {
		pos0.position.y += vert_overlap / 2.f;
		pos1.position.y -= vert_overlap / 2.f;
	}
	if (vel1.velocity.x < vel0.velocity.x) {
		pos0.position.x -= horiz_overlap / 2.f;
		pos1.position.x += horiz_overlap / 2.f;
	}
	else if (vel1.velocity.x > vel0.velocity.x) {
		pos0.position.x += horiz_overlap / 2.f;
		pos1.position.x -= horiz_overlap / 2.f;
	}
}

void WorldSystem::deal_damage(Entity& attacker, Entity& attacked, bool is_special, float damage_multiplier) {
	if (attacker == attacked) return; // Don't allow an entity to hit itself


	Living attacker_living = registry.livings.get(attacker);
	Living& attacked_living = registry.livings.get(attacked);

	if (attacked_living.invincible_time > 0.0) return; // cannot hit attacked


	if (registry.players.has(attacker)) {
		Player& player = registry.players.get(attacker);

		attacker_living.attack += player.attack_bonus.flat_bonus;
		attacker_living.attack *= (1.0f + player.attack_bonus.percent_bonus);

		attacker_living.special_attack += player.special_attack_bonus.flat_bonus;
		attacker_living.special_attack *= (1.0f + player.special_attack_bonus.percent_bonus);

		attacker_living.crit_rate += player.crit_rate_bonus;
		attacker_living.crit_damage += player.crit_damage_bonus;
	}

	float defense = attacked_living.defense;
	if (registry.players.has(attacked)) {
		Player& player = registry.players.get(attacked);

		defense += player.defense_bonus.flat_bonus;
		defense *= (1.0f + player.defense_bonus.percent_bonus);
	}

	float damage = 0.0f;
	if (is_special) damage = attacker_living.special_attack;
	else damage = attacker_living.attack;

	if (damage <= 0.f) return; // Do not apply anything if no damage is done

	damage *= damage_multiplier;

	// if random number generated is at or below crit rate * 100, apply crit dmg
	int random_number = rand() % 100 + 1;
	if (attacked_living.crit_rate * 100 >= random_number) {
		float crit_dmg_bonus =  attacked_living.crit_damage;

		crit_dmg_bonus = max(crit_dmg_bonus, 1.f); // Crits cannot lower your damage

		damage *= crit_dmg_bonus;

	}

	defense = max(defense, 0.f);
	damage = max(damage, 0.f);

	//std::cout << "before: " << enemy_health << std::endl;
	attacked_living.current_health -= damage * 100 / (100 + defense);
	//std::cout << "after: " << enemy_health << std::endl;

	attacked_living.invincible_time = 500.0;

	if (attacked_living.current_health <= 0) {

		if (registry.enemies.has(attacked)) {
			handle_enemy_killed(attacked);

			if (registry.enemies.size() == 0) {
				handle_all_enemies_killed();
			}
		}
	}

	// enemies react to taking damage
	if (registry.enemies.has(attacked)) {
		enemy_system->react_to_damage(attacked);
	}
	
	Entity& healthbar_entity = registry.healthbar.entities[0];
	HealthBar& healthbar = registry.healthbar.get(healthbar_entity);

	if (registry.players.has(attacked)) {
		healthbar.health = attacked_living.current_health;
	}
}

// [4] check if a point p1 is on the correct side of line
bool same_side(vec2 p1, vec2 p2, vec2 a, vec2 b) {
	// convert points to 3d since cross product is for 3d vectors
	vec3 p1_3d(p1.x, p1.y, 0.0);
	vec3 p2_3d(p2.x, p2.y, 0.0);
	vec3 a_3d(a.x, a.y, 0.0);
	vec3 b_3d(b.x, b.y, 0.0);

	vec3 cross_product_1 = cross(b_3d - a_3d, p1_3d - a_3d);
	vec3 cross_product_2 = cross(b_3d - a_3d, p2_3d - a_3d);
	float dot_product = dot(cross_product_1, cross_product_2);

	if (dot_product >= 0.0) {
		return true;
	}
	return false;
}

// check if point p is in triangle
bool point_in_triangle(vec2 p, vec2 a, vec2 b, vec2 c) {
	if (same_side(p, a, b, c) && same_side(p, b, a, c) && same_side(p, c, a, b)) {
		return true;
	}
	return false;
}

// detect collisions between one entity and another
bool WorldSystem::collide_triangle(Entity& entity, Entity& other) {
	if (!registry.enemies.has(other)) {
		return false;
	}

	Position& pos = registry.positions.get(other);

	vec2 top_left = { pos.position.x - pos.scale.x / 2.f, pos.position.y - pos.scale.y / 2.f };
	vec2 top_right = { pos.position.x + pos.scale.x / 2.f, pos.position.y - pos.scale.y / 2.f };
	vec2 bottom_left = { pos.position.x - pos.scale.x / 2.f, pos.position.y + pos.scale.y / 2.f };
	vec2 bottom_right = { pos.position.x + pos.scale.x / 2.f, pos.position.y + pos.scale.y / 2.f };

	// for triangle vertices in entity, check if vertex is in triangle mesh of other
	std::vector<Triangle> entity_triangles = registry.triangleMesh.get(entity).triangle_mesh;

	// for vertex in entity triangle, check if it's in triangle of other
	for (const Triangle& entity_triangle : entity_triangles) {
		for (const vec2& vertex : entity_triangle.vertices) {

			if (point_in_triangle(vertex, top_left, top_right, bottom_left) || point_in_triangle(vertex, top_right, bottom_right, bottom_left)) {
				return true;
			}
		}
	}

	return false;
}

void WorldSystem::handle_enemy_killed(Entity enemy_entity) {
	Position& pos = registry.positions.get(enemy_entity);
	Enemy& e = registry.enemies.get(enemy_entity);

	Entity screen_state_entity = renderer->get_screen_state_entity();
	ScreenState& screen_state = registry.screenStates.get(screen_state_entity);
	
	int num_drops = e.souls_value;
	int num_health_drops = e.health_value;

	if (screen_state.tutorialStep == 9) {
		num_drops = 0;
	}

	for (int i = 0; i < 100; i++) {
		float angle = 2 * M_PI * rand() / RAND_MAX;
		float speed = BASE_TILE_SIZE_WIDTH;
		vec2 v = { speed * cos(angle), speed * sin(angle) };
		particle_system->createParticle(pos.position, { 10, 10 }, v * 2.0f, -v * 1.5f, 1000.0, PARTICLE_TEXTURE_ID::TEST_PARTICLE, ACCELERATED);
	}
	
	for (int i = 0; i < num_drops; i++) {
		float angle = rand() * M_PI * 2.f / 180.f;
		float spread = rand() * ITEM_PICKUP_MAX_SPREAD_DISTANCE / RAND_MAX;

		vec2 start_position = pos.position;
		vec2 end_position = start_position + vec2(cos(angle), sin(angle)) * ITEM_PICKUP_MAX_SPREAD_DISTANCE;

		item_system->createPickupItem(SOULS, start_position, end_position, 1.0f); // TODO: get soul count from enemy
	}

	// M4 New Items (health drop)
	for (int i = 0; i < num_health_drops; i++) {

		float random_percent = rand() % 101;

		if (random_percent <= HEALTH_DROP_SPAWN_PERCENT) {

			float angle = rand() * M_PI * 2.f / 180.f;
			float spread = rand() * ITEM_PICKUP_MAX_SPREAD_DISTANCE / RAND_MAX;

			vec2 start_position = pos.position;
			vec2 end_position = start_position + vec2(cos(angle), sin(angle)) * ITEM_PICKUP_MAX_SPREAD_DISTANCE;

			item_system->createPickupItem(HEALTH, start_position, end_position, HEALTH_DROP_HEAL_PERCENT);
		}
	}

	enemy_system->handle_enemy_death(enemy_entity);
}

void WorldSystem::handle_all_enemies_killed() {
	// TODO: REMOVE AFTER ITEMS ARE MORE IMPLEMENTED

	Entity screen_state_entity = renderer->get_screen_state_entity();
	ScreenState& screen_state = registry.screenStates.get(screen_state_entity);

	for (Entity e : registry.itemSpawns.entities) {
		Position& p = registry.positions.get(e);

		if (screen_state.tutorialStep != 6) {
			createCollidableItem(item_system->get_random_item(), p.position);
		}

	}
}

// Sword collision resolution
void WorldSystem::handle_sword_collision(Entity& sword_entity, Entity& other) {
	if (registry.livings.has(other)) {
		Attack& attack = registry.attacks.get(sword_entity);

		// only deal damage if triangle points of sword are in triangle meshes of other
		if (collide_triangle(sword_entity, other)) {
			deal_damage(attack.attacker, other, false, 1.f);
		}
	}
}

void WorldSystem::handle_projectile_collision(Entity& projectile_entity, Entity& other) {
	if (registry.livings.has(other)) {
		Attack& attack = registry.attacks.get(projectile_entity);
		Projectile& projectile = registry.projectiles.get(projectile_entity);

		if ((registry.enemies.has(other) && registry.players.has(attack.attacker))
			|| registry.players.has(other) && registry.enemies.has(attack.attacker)) {
			deal_damage(attack.attacker, other, true, projectile.damage_multiplier);
			registry.remove_all_components_of(projectile_entity);
		}
		
	}
	else if (registry.mapTiles.has(other)) {
		registry.remove_all_components_of(projectile_entity);
	}
}

void WorldSystem::handle_enemy_attack_collision(Entity& enemy_entity, Entity& other) {
	if (!registry.enemies.has(enemy_entity)) return;
	if (registry.enemies.get(enemy_entity).current_state != ENEMY_ATTACKING) return;
	deal_damage(enemy_entity, other, false, 1.f);
}

void WorldSystem::handle_exit_collision() {

	// unload map and load new map
	map_loader.unloadCurrentMap(renderer, ai_system);

	if (screen_manager->getCurrentScreen() == ScreenType::TutorialScreen) {
		screen_manager->endTutorial();
		restart_game();
		return;
	}
	else {
		map_loader.loadNextMap(renderer, ai_system);
	}

	// set player position
	Entity& player_entity = registry.players.entities[0];
	Position& player_pos = registry.positions.get(player_entity);

	vec2 entrance_pos;

	if (registry.entrance.size() == 0) {
		entrance_pos = Util::get_position_from_coord(vec2(2, 2));
	}
	else {
		Entity& entrance_entity = registry.entrance.entities[0];
		entrance_pos = registry.positions.get(entrance_entity).position;
	}

	
	player_pos.position.x = entrance_pos.x + BASE_TILE_SIZE_WIDTH;
	player_pos.position.y = entrance_pos.y;

	int enemy_spawn_type;

	//if (registry.enemySpawns.size() > 0) {
	//	rewards_spawned = false;
	//}


	// spawn enemies at spawn locations
	for (Entity spawn : registry.enemySpawns.entities) {
		ENEMY_TYPE type = DEBUG_ENEMY;
		if (map_loader.getLevel() % BOSS_ROOM_LEVEL == 0) {
			type = BOSS;
		}
		else {
			enemy_spawn_type = rand() % 2;
			if (enemy_spawn_type > 0) {
				type = RANGED;
			}
			else {
				type = CHARGER;
			}
		}
		Position& spawnPos = registry.positions.get(spawn);
		this->enemy_system->create_enemy(*renderer, spawnPos.position, type);
	}

	return;
}

// handle parry collision
void WorldSystem::handle_parry_collision(Entity& parry_entity, Entity& other) {
	Entity other_attacker;

	// knockback enemy if parried
	if (registry.livings.has(other) && registry.enemies.has(other) && registry.attacks.has(other)) {
		enemy_system->react_to_parry(other);
	}
	// delet enemy projectile if parried
	else if (registry.projectiles.has(other) && registry.attacks.has(other)) {
		other_attacker = registry.attacks.get(other).attacker;
		// delete projectile if owner of projectile is enemy and collision is detected
		if (registry.enemies.has(other_attacker)) {
			registry.remove_all_components_of(other);
		}
	}
}

// Collision handling

// Compute collisions between entities
void WorldSystem::handle_collisions() {

	ComponentContainer<Collision>& collision_container = registry.collisions;
	for (uint i = 0; i < collision_container.components.size(); i++) {
		
		Entity& first = collision_container.entities[i];
		Entity& second = collision_container.components[i].other;

		if (registry.mapTiles.has(first) || registry.mapTiles.has(second)) {
			Entity& mapTile = registry.mapTiles.has(first) ? first : second;
			Entity&  other = !registry.mapTiles.has(first) ? first : second;
			
			if (registry.mapTiles.get(mapTile).exit && registry.enemies.entities.size() == 0 && registry.players.has(other)) {
				handle_exit_collision();
				return;
			}

			handle_tile_collision(mapTile, other);
		}

		if (registry.swords.has(first) || registry.swords.has(second)) {
			Entity& sword_entity = registry.swords.has(first) ? first : second;
			Entity& other = !registry.swords.has(first) ? first : second;
			handle_sword_collision(sword_entity, other);
		}

		if (registry.projectiles.has(first) || registry.projectiles.has(second)) {
			Entity& projectile_entity = registry.projectiles.has(first) ? first : second;
			Entity& other = !registry.projectiles.has(first) ? first : second;

			handle_projectile_collision(projectile_entity, other);

		}

		if (registry.parries.has(first) || registry.parries.has(second)) {
			Entity& parry_entity = registry.parries.has(first) ? first : second;
			Entity& other = !registry.parries.has(first) ? first : second;

			handle_parry_collision(parry_entity, other);
		}

		if ((registry.attacks.has(first) && registry.players.has(second))
			|| (registry.players.has(first) && registry.attacks.has(second))) {
			Entity& enemy_entity = registry.attacks.has(first) ? registry.attacks.get(first).attacker : registry.attacks.get(second).attacker;
			Entity& other = !registry.attacks.has(first) ? first : second;
			//std::cerr << "enemy attack collision";
			if (!registry.enemies.has(enemy_entity)) continue;
			handle_enemy_attack_collision(enemy_entity, other);
		}

		if (registry.items.has(first) || registry.items.has(second)) {
			Entity& item_entity = registry.items.has(first) ? first : second;
			Entity& other = !registry.items.has(first) ? first : second;

			// For New Feature: Item Stat Block
			if (registry.players.has(other)) {
				Item& item = registry.items.get(item_entity);

				// Check for souls
				if (!item.is_pickup) {
					Entity screen_state_entity = renderer->get_screen_state_entity();
    				ScreenState& screen_state = registry.screenStates.get(screen_state_entity);
					vec2 item_pos = registry.positions.get(item_entity).position;
					selected_item_entity = &item_entity;
					is_itempopup_visible = true;
					screen_state.item_position = item_pos;
					renderer->drawItemStat(item_entity);
				}

				handle_item_player_collision(item_entity, other);
			}
		}
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

void WorldSystem::handle_item_player_collision(Entity& item_entity, Entity& player_entity) {
	Item& item = registry.items.get(item_entity);

	if (item.is_pickup && item.can_pickup) {
		player_system->pickup_item(item_entity);
		Mix_PlayChannel(-1, souls_pickup, 0);
		registry.remove_all_components_of(item_entity);
	}
	else {
		selected_item_entity = &item_entity;
	}
}

void WorldSystem::spawn_enemy_pack() {
	Position& player_position = registry.positions.get(registry.players.entities[0]);

	Entity screen_state_entity = renderer->get_screen_state_entity();
    ScreenState& screen_state = registry.screenStates.get(screen_state_entity);

	if (screen_state.tutorialActive) {
		enemy_system->create_enemy(*renderer, Util::get_position_from_coord(vec2(9, 3)), CHARGER);
	}
	else if (player_position.position.y < Util::get_position_from_coord(vec2(3, 4)).y) {
		enemy_system->create_enemy(*renderer, Util::get_position_from_coord(vec2(3, 6)), CHARGER);
		enemy_system->create_enemy(*renderer, Util::get_position_from_coord(vec2(6, 6)), CHARGER);
		enemy_system->create_enemy(*renderer, Util::get_position_from_coord(vec2(9, 6)), CHARGER);
	}
	else {
		enemy_system->create_enemy(*renderer, Util::get_position_from_coord(vec2(3, 2)), CHARGER);
		enemy_system->create_enemy(*renderer, Util::get_position_from_coord(vec2(6, 2)), CHARGER);
		enemy_system->create_enemy(*renderer, Util::get_position_from_coord(vec2(9, 2)), CHARGER);
	}
	
}

// Handle healthbar position
void WorldSystem::updateHealthbarPosition(Entity e, vec2 camera_position) {
	HealthBar& health_bar = registry.healthbar.get(e);
	// adjust health

	Position& p = registry.positions.get(e);
	float initial_xpos = HEALTHBAR_XPOS;
	float initial_ypos = HEALTHBAR_YPOS;
	p.position.x = initial_xpos + p.scale.x / 2 + camera_position.x;
	p.position.y = initial_ypos + camera_position.y;
}

// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

bool WorldSystem::is_player_alive() {
	Entity& player_entity = registry.players.entities[0];
	Living& player_living = registry.livings.get(player_entity);
	if (player_living.current_health <= 0.0) {
		return false;
	}
	return true;
}

// on key callback
void WorldSystem::on_key(int key, int, int action, int mod) {


	if (action == GLFW_RELEASE && key == config.key_bindings.interact) {
		if (selected_item_entity != NULL) {
			player_system->pickup_item(*selected_item_entity);
			Mix_PlayChannel(-1, item_equip, 0);
			registry.remove_all_components_of(*selected_item_entity);
			is_itempopup_visible = false;

			std::vector<Entity> to_remove;

			for (Entity e : registry.items.entities) {
				if (!registry.items.get(e).is_pickup) to_remove.push_back(e);
			}

			for (Entity e : to_remove) {
				registry.remove_all_components_of(e);
			}

			selected_item_entity = NULL;
		}
	}

	// only perform special if special_timer is 0
	Entity& player_entity = registry.players.entities[0];
	Player& player = registry.players.get(player_entity);
	if (key == config.key_bindings.special_q && player.special_timer <= 0.0 || 
		key == config.key_bindings.special_e && player.special_timer <= 0.0) {
		player_system->do_special(vec2(mouse_pos_x, mouse_pos_y));
		Mix_PlayChannel(-1, parry, 0);
	}
	player_system->on_key(key, action, mod);

	if (player.current_state == PLAYER_DASHING) {
		Mix_PlayChannel(-1, dash, 0);
	}
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {

	// record the current mouse position
	ScreenState& screen_state = registry.screenStates.components[0];
	vec2 camera_pos = screen_state.camera_position;
	camera_pos -= vec2(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX) / 2.0f;
	mouse_pos_x = mouse_position.x + camera_pos.x;
	mouse_pos_y = mouse_position.y + camera_pos.y;
}

void WorldSystem::on_mouse_button_pressed(int button, int action, int mods) {

	// on button press
	// Mouse Control
	if (screen_manager->getCurrentScreen() == ScreenType::PlayScreen || screen_manager->getCurrentScreen() == ScreenType::TutorialScreen) {
		if (action == GLFW_PRESS) {

			int tile_x = (int)(mouse_pos_x / GRID_CELL_WIDTH_PX);
			int tile_y = (int)(mouse_pos_y / GRID_CELL_HEIGHT_PX);
	
			std::cout << "mouse position: " << mouse_pos_x << ", " << mouse_pos_y << std::endl;
			std::cout << "mouse tile position: " << tile_x << ", " << tile_y << std::endl;
	
			// only swing if sword_timer is 0
			Entity& player_entity = registry.players.entities[0];
			Player& player = registry.players.get(player_entity);
			if (button == GLFW_MOUSE_BUTTON_LEFT && player.sword_timer <= 0.0 && screen_manager->getCurrentTutorialStep() != 7 
				&& screen_manager->getCurrentTutorialStep() != 9 && screen_manager->getCurrentTutorialStep() != 10) {
				player_system->do_melee_attack(vec2(mouse_pos_x, mouse_pos_y));
				Mix_PlayChannel(-1, sword_swing_sound, 0);
		}
			if (button == GLFW_MOUSE_BUTTON_RIGHT && screen_manager->getCurrentTutorialStep() != 6 && screen_manager->getCurrentTutorialStep() != 7) {
				player_system->start_charging();
			}
		}
		else if (action == GLFW_RELEASE && screen_manager->getCurrentTutorialStep() != 6 && screen_manager->getCurrentTutorialStep() != 7) {
			if (button == GLFW_MOUSE_BUTTON_RIGHT) {
				if (player_system->do_ranged_attack(vec2(mouse_pos_x, mouse_pos_y))) Mix_PlayChannel(-1, laser_sound, 0);
			}
		}
	} 
}

// M3 Creative Component: Reloadability
void WorldSystem::save_game_state() {
	assert(registry.gameStates.size() > 0);

	GameState& game_state = registry.gameStates.components[0];

	nlohmann::json save_game;

	save_game["souls"] = game_state.souls;

	std::vector<std::string> unlocked_upgrades;
	for (std::string str : game_state.unlocked_upgrades) {
		unlocked_upgrades.push_back(str);
	}

	save_game["unlocked_upgrades"] = unlocked_upgrades;

	std::ofstream f(json_path("save_game.json"));

	f << save_game;
}

// M3 Creative Component: Reloadability
void WorldSystem::load_game_state() {
	assert(registry.gameStates.size() > 0);

	GameState& game_state = registry.gameStates.components[0];
	game_state.souls = 0;
	game_state.unlocked_upgrades.clear();

	try {
		std::ifstream f(json_path("save_game.json"));
		auto data = nlohmann::json::parse(f);

		game_state.souls = data["souls"];
		game_state.unlocked_upgrades = data["unlocked_upgrades"];
	}
	catch (...) {
		std::cerr << "There was a problem reading from the save file, using default values" << std::endl;
	}
}