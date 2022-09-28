
#include "common.h"

#include "sparkles.h"
#include "sparkles_utils.h"
using namespace Sparkles;



RandomVec2 spawn_position_spec;
ParticleSystem* water_particles;

// #temporary: Remove these.
#define min(a, b) (a < b) ? (a) : (b)
#define max(a, b) (a > b) ? (a) : (b)


PhysicsParams physics = {
	.gravity = {0, -2},
	.friction = 1,
};

SpawnParams spawn = {
	.position = {
		.coordinate_system = CoordinateSystem::POLAR, 
		.angle  = {Distribution::UNIFORM, 0, TAU},
		.radius = {Distribution::UNIFORM, 0, 0.05},
	},
	
	.scale = {Distribution::UNIFORM, 0.004, 0.05},
	
	.color = {
		.color_system = ColorSystem::RGB,
		.red   = {Distribution::UNIFORM, 0, 0.2},
		.green = {Distribution::UNIFORM, 0.7, 0.9},
		.blue  = {Distribution::UNIFORM, 0.9, 1},
		.alpha = {Distribution::UNIFORM, 0.2, 0.4},
	},
	
	.velocity = {
		.coordinate_system = CoordinateSystem::POLAR,
		.angle =  {Distribution::UNIFORM, +0.05 * TAU, +0.1 * TAU},
		.radius = {Distribution::UNIFORM, 3, 5},
	},
	
	.life = {Distribution::UNIFORM, 0.1, 3},
};

float particles_per_second = 1000;
float accumulated_remaining_particles_to_spawn = 0;

void waterfall_init() {
	water_particles = particle_system_create(10000);
}

void waterfall_frame(float dt) {
	accumulated_remaining_particles_to_spawn += particles_per_second * dt;

	// Spawn new water particles.
	for (int i = 0; i < water_particles->count && accumulated_remaining_particles_to_spawn >= 1; i += 1) {
		Particle* p = &water_particles->particles[i];
		
		if (p->life < 0) {
			particle_spawn(p, &spawn);
			p->position += {-0.5, +0.3};
			accumulated_remaining_particles_to_spawn -= 1;
		}
	}
	
	// Simulate water particles
	for (int i = 0; i < water_particles->count; i += 1) {
		Particle* p = &water_particles->particles[i];
		particle_simulate(p, &physics, dt);
		p->color.w = min(p->life, p->color.w);
	}
	
	// Upload and render
	RenderState render_state;
	render_state.projection = orthographic(-0.8f, +0.8f, +0.5f, -0.5f, -1, +1);
	particle_system_upload_and_render(water_particles, circle_mesh, &render_state);
	
	// UI controls
	ImGui::SliderFloat("Particles per second", &particles_per_second, 0.0f, 5000.0f, "%.0f");
	
}