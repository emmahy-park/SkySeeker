#include "particle_system.hpp"
#include <vector>
#include <glm/trigonometric.hpp>

void ParticleSystem::step(float elapsed_ms) {

	std::vector<Entity> to_remove;
	for (Entity e : registry.particles.entities) {
		Particle &p = registry.particles.get(e);
		p.life_time -= elapsed_ms;
		particle_handlers[p.type](e, elapsed_ms);
		if (p.life_time <= 0) {
			to_remove.push_back(e);
		}
	}
	while (to_remove.size() > 0) {
		registry.remove_all_components_of(to_remove.back());
		to_remove.pop_back();
	}


}

void ParticleSystem::linear_partical_handler(Entity& e, float elapsed_ms) {
	Position& p = registry.positions.get(e);
	Particle particle = registry.particles.get(e);
	// Will need to clarify if we're setting rotation then velocity based on that or if velocity determines the movement of particle
	p.position = p.position + particle.velocity * elapsed_ms / 1000.0f;
}

void ParticleSystem::acceleration_partical_handler(Entity& e, float elapsed_ms) {
	Position& p = registry.positions.get(e);
	Particle& particle = registry.particles.get(e);
	// Will need to clarify if we're setting rotation then velocity based on that or if velocity determines the movement of particle
	particle.velocity += particle.acceleration * elapsed_ms / 1000.0f;
	p.position = p.position + particle.velocity * elapsed_ms / 1000.0f;
}

void ParticleSystem::getParticleTransforms(std::vector<mat3> &transforms) {
	for (Entity e : registry.particles.entities) {
		Position position = registry.positions.get(e);
		Transform transform;
		transform.translate(position.position);
		transform.scale(position.scale);
		transform.rotate(radians(position.angle));
		transforms.push_back(transform.mat);
	}
}

void ParticleSystem::init(RenderSystem& renderer) {

	renderer.loadGlTextures(particle_texture_handles.data(), particle_paths.data(), particle_paths.size());

}

Entity ParticleSystem::createParticle(vec2 position, vec2 scale, vec2 velocity, vec2 acceleration, float life_time, PARTICLE_TEXTURE_ID particle_texture_id, PARTICLE_TYPE particle_type) {
	Entity e = Entity();
	Particle& particle = registry.particles.emplace(e);
	particle.type = particle_type;
	particle.life_time = life_time;
	particle.acceleration = acceleration;
	particle.velocity = velocity;

	Position& pos = registry.positions.emplace(e);
	pos.position = position;
	pos.scale = scale;
	pos.layer = 0;

	TextureInfo& textureInfo = registry.textureinfos.emplace(e);
	textureInfo.texture_id = particle_texture_handles[(int)particle_texture_id];
	textureInfo.bottom_right = { 1, 1 };
	textureInfo.top_left = { 0, 0 };

	return e;
}