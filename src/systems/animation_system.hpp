#pragma once

#include "common.hpp"
#include "../tinyECS/tiny_ecs.hpp"
#include "../tinyECS/components.hpp"
#include "../tinyECS/registry.hpp"

// A simple physics system that moves rigid bodies and checks for collision
class AnimationSystem
{
public:

	typedef Entity (*AnimationHandler)(Entity& e, Animation& a, float elapsed_ms);

	static Entity linear_animation_handler(Entity& e, Animation& a, float elapsed_ms);

	static Entity step_animation_handler(Entity& e, Animation& a, float elapsed_ms);

	static Entity single_linear_animation_handler(Entity& e, Animation& a, float elapsed_ms);

	static inline AnimationHandler animation_handlers[] = {
		linear_animation_handler, step_animation_handler, single_linear_animation_handler
	};

	void step(float elapsed_ms);


	AnimationSystem()
	{
	}
};