
#pragma once

#include "common.hpp"
#include "render_system.hpp"
#include "../tinyECS/tiny_ecs.hpp"
#include "../tinyECS/components.hpp"
#include "../tinyECS/registry.hpp"

class ParticleSystem {
	const std::array<std::string, particle_count> particle_paths = {
		textures_path("particles/spawn_tile.png"),
		textures_path("particles/white_bubble.png")
	};
	std::array<GLuint, particle_count> particle_texture_handles;
public:

	void getParticleTransforms(std::vector<mat3> &transforms);
	void step(float elapsed_ms);

	typedef void(*ParticleHandler)(Entity& e, float elapsed_ms);

	static void linear_partical_handler(Entity& e, float elapsed_ms);

	static void acceleration_partical_handler(Entity& e, float elapsed_ms);

	static inline ParticleHandler particle_handlers[] = {
		linear_partical_handler,
		acceleration_partical_handler
	};

	void init(RenderSystem& renderer);

	Entity createParticle(vec2 position, vec2 scale, vec2 velocity, vec2 acceleration, float life_time, PARTICLE_TEXTURE_ID particle_texture_id, PARTICLE_TYPE particle_type);

};