#pragma once

#include "common.hpp"

// Keybindings
struct Keybindings {
	int move_left = GLFW_KEY_A;
	int move_right = GLFW_KEY_D;
	int move_up = GLFW_KEY_W;
	int move_down = GLFW_KEY_S;
	int dash = GLFW_KEY_SPACE;

	int interact = GLFW_KEY_F;

	int special_q = GLFW_KEY_Q;
	int special_e = GLFW_KEY_E;
};

struct Config {
	Keybindings key_bindings;
};

static inline Config config = Config();