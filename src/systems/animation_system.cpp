#include "animation_system.hpp"

// [2] Sprite Animation
void AnimationSystem::step(float elapsed_ms) {
	auto& animation_registry = registry.animations;
	for (int i = 0; i < animation_registry.size(); i++) {
		Animation& animation = animation_registry.components[i];
		Entity& entity = animation_registry.entities[i];

		animation_handlers[animation.type](entity, animation, elapsed_ms);
	}
}

/*
	TODO: Implement me
*/
Entity AnimationSystem::linear_animation_handler(Entity& e, Animation& a, float elapsed_ms) {
	a.info.la.animation_ms += elapsed_ms;
	a.current_frame = (int)(a.info.la.animation_ms / a.info.la.animation_step) % a.max_frames;
	return e;
}

/*
	TODO: Implement
*/
Entity AnimationSystem::step_animation_handler(Entity& e, Animation& a, float elapsed_ms) {
	return e;
}

Entity AnimationSystem::single_linear_animation_handler(Entity& e, Animation& a, float elapsed_ms) {
	a.info.sla.animation_ms += elapsed_ms;
	a.current_frame = (int)(a.info.sla.animation_ms / a.info.sla.animation_step);
	
	if (a.current_frame >= a.max_frames) {
		a.info.sla.callback_fn(e); // This callback_fn should change the animation.
	}

	return e;
}