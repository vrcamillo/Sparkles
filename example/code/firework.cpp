#include "common.h"

#include "sparkles.h"
#include "sparkles_utils.h"
using namespace Sparkles;

// Particle parameters
ParticleSystem* system;

struct Params {
	RandomVec2   explosion_position = uniform2_rect({-0.05, -0.05}, {+0.05, +0.05});
	RandomScalar explosion_interval = uniform1(0.25, 1);
	RandomScalar particles_per_explosion = uniform1(50, 100);
	
	ParticlePhysicsParams physics = {
		.gravity = {0, -1},
		.friction = 0.99,
	};
	
	ParticleSpawnParams spawn = {
		.position = uniform2_polar(0, TAU, 0, 0.1),
		.scale    = uniform1(0.01, 0.10),
		.color    = uniform_rgba({0.8, 0.5, 0.5, 0.8}, {2, 1.5, 1.5, 1}),
		.velocity = uniform2_polar(0, TAU, 3, 5),
		.life     = uniform1(0.5, 2),
	};
};

Params params;

float explosion_accumulation_timer;
float next_explosion_interval;

RenderTarget* hdr_render_target;
int render_target_width;
int render_target_height;

Shader* hdr_pixel_shader;

Texture* light_mask;

void firework_init() {	
	system = particle_system_create(1000);
	
	for (int i = 0; i < system->count; i += 1) {
		Particle* p = &system->particles[i];
		particle_spawn(p, &params.spawn);
		p->scale = 0;
	}
	
	next_explosion_interval = random_get(params.explosion_interval);
	
	// Create our hdr render target the same size as our backbuffer.
	glfwGetFramebufferSize(global_window, &render_target_width, &render_target_height);
	hdr_render_target = render_target_create(TextureFormat::RGBA_FLOAT16, render_target_width, render_target_height);
	
	light_mask = texture_generate_light_mask(32, 32, 0.01, 10);
}

void firework_frame(float dt) {
	explosion_accumulation_timer += dt;
	
	if (explosion_accumulation_timer > next_explosion_interval) {
		vec2 spawn_position = random_get(params.explosion_position);
		int remaining_particles_to_spawn = (int) random_get(params.particles_per_explosion);
		
		for (int i = 0; i < system->count && remaining_particles_to_spawn > 0; i += 1) {
			Particle* p = &system->particles[i];
			if (p->life < 0) {
				particle_spawn(p, &params.spawn);
				p->position.xy += spawn_position;
				remaining_particles_to_spawn -= 1;
			}
		}
		
		next_explosion_interval = random_get(params.explosion_interval);
		explosion_accumulation_timer = 0;
	}
	
	for (int i = 0; i < system->count; i += 1) {
		Particle* p = &system->particles[i];
		particle_simulate(p, &params.physics, dt);
		
		if (p->life < 0) p->scale = 0;
		
		p->color.w = fmin(p->color.w, p->life);
	}
	
	RenderState render_state;
	render_state.render_target = hdr_render_target;
	render_state.projection = orthographic(-0.8f, +0.8f, +0.5f, -0.5f, -1, +1);
	render_state.viewport = {0, 0, (float) render_target_width, (float) render_target_height};
	render_state.texture0 = light_mask;
	
	render_target_clear(hdr_render_target, {0, 0, 0, 1});
	particle_system_upload_and_render(system, square_mesh, &render_state);
	
	auto hdr_texture = render_target_flush(hdr_render_target);
	
	render_state.render_target = nullptr;
	render_state.projection = orthographic(-0.5f, +0.5f, +0.5f, -0.5f, -1, +1);;
	render_state.viewport = {0, 0, (float) render_target_width, (float) render_target_height};
	render_state.texture0 = hdr_texture;
	mesh_render(circle_mesh, &render_state);
	
}