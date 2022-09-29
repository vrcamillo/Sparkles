#include "common.h"

#include "sparkles.h"
#include "sparkles_utils.h"
using namespace Sparkles;

// Particle parameters
ParticleSystem* system;

struct Params {
	RandomVec2   explosion_position = uniform2_rect({-0.01, -0.01}, {+0.01, +0.01});
	RandomScalar explosion_interval = uniform1(2, 3);
	RandomScalar particles_per_explosion = uniform1(300, 700);
	
	ParticlePhysicsParams physics = {
		.gravity = {0, -0.2},
		.friction = 0.99,
	};
	
	ParticleSpawnParams spawn = {
		.position = uniform2_polar(0, TAU, 0, 0.01),
		.scale    = uniform1(0.01, 0.10),
		.color    = uniform_rgba({0.8, 0.2, 0.2, 1}, {1, 0.4, 0.4, 2}),
		.velocity = uniform2_polar(0, TAU, 0, 2),
		.life     = uniform1(0.5, 2),
	};
};

Params params;

float explosion_accumulation_timer;
float next_explosion_interval;

RenderTarget* hdr_render_target;
int render_target_width;
int render_target_height;

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
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE); // #temporary
	render_target_clear(hdr_render_target, {0, 0, 0, 1});
	particle_system_upload_and_render(system, circle_mesh, &render_state);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // #temporary: Reset default opengl state

	
	auto hdr_texture = render_target_flush(hdr_render_target);
	
	render_state.render_target = nullptr;
	render_state.projection = orthographic(-0.5f, +0.5f, +0.5f, -0.5f, -1, +1);;
	render_state.viewport = {0, 0, (float) render_target_width, (float) render_target_height};
	render_state.texture0 = hdr_texture;
	mesh_render(square_mesh, &render_state);
	
	ImGui::RandomEdit("Number of particles", &params.particles_per_explosion, 0, 1000);
	ImGui::RandomEdit("Interval", &params.explosion_interval, 1, 10);
	ImGui::RandomEdit("Color", &params.spawn.color);
	ImGui::RandomEdit("Life", &params.spawn.life, 0, 5);
	ImGui::RandomEdit("Size", &params.spawn.scale, 0, 1);
	
	ImGui::Separator();
	
	ImGui::InputFloat("Gravity", &params.physics.gravity.y, -1, 1);
	ImGui::SliderFloat("Friction", &params.physics.friction, 0, 1);
	
}