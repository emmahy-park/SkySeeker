
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stdlib
#include <chrono>
#include <iostream>

// internal
#include "systems/ai_system.hpp"
#include "systems/physics_system.hpp"
#include "systems/render_system.hpp"
#include "systems/world_system.hpp"
#include "systems/animation_system.hpp"
#include "systems/upgrade_system.hpp"
#include "util/map_parser.hpp"
#include "util/screen_manager.hpp"
#include "systems/particle_system.hpp"

// imgui
#include "../ext/imgui/imgui.h"
#include "../ext/imgui/imgui_impl_glfw.h"
#include "../ext/imgui/imgui_impl_opengl3.h"

using Clock = std::chrono::high_resolution_clock;

// Entry point
int main()
{
	// global systems
	AISystem	  	ai_system;
	WorldSystem   	world_system;
	RenderSystem  	renderer_system;
	PhysicsSystem 	physics_system;
	PlayerSystem  	player_system;
	EnemySystem   	enemy_system;
	AnimationSystem animation_system;
	ItemSystem		item_system;
	ParticleSystem	particle_system;
	UpgradeSystem   upgrade_system;
	ScreenManager 	screen_manager(&renderer_system, &world_system, &upgrade_system);

	// initialize window
	GLFWwindow* window = world_system.create_window();
	if (!window) {
		// Time to read the error message
		std::cerr << "ERROR: Failed to create window.  Press any key to exit" << std::endl;
		getchar();
		return EXIT_FAILURE;
	}

	if (!world_system.start_and_load_sounds()) {
		std::cerr << "ERROR: Failed to start or load sounds." << std::endl;
	}

	// initialize the main systems
	renderer_system.init(window);
	renderer_system.fontInit(window);
	renderer_system.initImGui(window);
	player_system.init(renderer_system);
	enemy_system.init(renderer_system);
	item_system.init(renderer_system);
	upgrade_system.init(renderer_system);
	particle_system.init(renderer_system);
	ai_system.init(&enemy_system);
	world_system.init(&renderer_system, &player_system, &enemy_system, &item_system, &screen_manager, &upgrade_system, &particle_system, &ai_system);

	//map_loader.parseMaps(textures_path("map/city_0.json"), &renderer_system);

	// variable timestep loop
	auto t = Clock::now();
	while (!world_system.is_over()) {
		
		// processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// calculate elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float actual_elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;

		float elapsed_ms = actual_elapsed_ms;

		if (actual_elapsed_ms > 33.3) {
			elapsed_ms = 33.3;
		}

		if (screen_manager.getCurrentScreen() == ScreenType::PlayScreen || screen_manager.getCurrentScreen() == ScreenType::TutorialScreen) {
			// CK: be mindful of the order of your systems and rearrange this list only if necessary
			animation_system.step(elapsed_ms); // Update animations before anything else.
											// This lets any further changes to animation happen immediately
			world_system.step(elapsed_ms, actual_elapsed_ms);
			player_system.step(elapsed_ms);
			ai_system.step(elapsed_ms);
			enemy_system.step(elapsed_ms);
			item_system.step(elapsed_ms);
			physics_system.step(elapsed_ms);
			particle_system.step(elapsed_ms);
			world_system.handle_collisions();
		}

		screen_manager.renderScreen();
		screen_manager.handleInput(window);

	}

	world_system.save_game_state();

	renderer_system.shutdown(window);
	
	return EXIT_SUCCESS;
}
