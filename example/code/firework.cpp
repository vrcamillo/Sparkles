#include "common.h"

#include "sparkles.h"
#include "sparkles_utils.h"
using namespace Sparkles;

//
// Particle system parameters 
//
ParticleSystem* system;
constexpr int max_particle_count = 1000;

Range2 explosion_position = {{-0.01, -0.01}, {+0.01, +0.01}};
Range1 explosion_interval = {0.5, 2};
Range1 particles_per_explosion = {300, 700};

vec2 gravity = {0, -0.2};
float friction = 0.99;

ParticleSpawnParams spawn_params = {
	.position = polar(0, 0.010, 0, TAU),
	.scale    = {0.01, 0.10},
	.color    = {{0.8, 0.2, 0.2, 1}, {1, 0.4, 0.4, 2}},
	.velocity = polar(0, 2, 0, TAU),
	.life     = {0.5, 2},
};

//
// Simulation variables
//
float explosion_accumulation_timer;
float next_explosion_interval = -1;

//
// Graphics variables
//
RenderTarget* hdr_render_target;
Texture* light_mask;
RenderState particles_render_state;
RenderState hdr_blit_render_state;


void firework_init() {
	system = particle_system_create(max_particle_count);

	// Initialize all particles.
	for (int i = 0; i < system->count; i += 1) {
		Particle* p = &system->particles[i];
		p->scale = 0;
		p->life = -1;
	}
	
	{
		//
		// Initialize our graphics variables.
		//
		
		// Create our HDR render target the same size as our backbuffer.
		int backbuffer_width, backbuffer_height;
		glfwGetFramebufferSize(global_window, &backbuffer_width, &backbuffer_height);
		hdr_render_target = render_target_create(TextureFormat::RGBA_FLOAT16, backbuffer_width, backbuffer_height);
		
		// Create our light texture mask.
		light_mask = texture_generate_light_mask(32, 32, 0.01, 10);
		
		particles_render_state.render_target = hdr_render_target;
		particles_render_state.projection = orthographic(-0.8f, +0.8f, +0.5f, -0.5f, -1, +1);
		particles_render_state.viewport = {0, 0, (float) hdr_render_target->width, (float) hdr_render_target->height};
		particles_render_state.texture0 = light_mask;
		
		hdr_blit_render_state.projection = orthographic(-0.5f, +0.5f, +0.5f, -0.5f, -1, +1);;
		hdr_blit_render_state.viewport = {0, 0, (float) backbuffer_width, (float) backbuffer_height};
		hdr_blit_render_state.texture0 = nullptr; // Will be our HDR texture.
		
	}
	
}

void firework_frame(float dt) {
	explosion_accumulation_timer += dt;
	
	if (next_explosion_interval < 0 || explosion_accumulation_timer > next_explosion_interval) {
		vec2 spawn_position = random_get2(explosion_position);
		
		int remaining_particles_to_spawn = (int) random_get1(particles_per_explosion);
		for (int i = 0; i < system->count && remaining_particles_to_spawn > 0; i += 1) {
			Particle* p = &system->particles[i];
			if (p->life < 0) {
				particle_spawn(p, &spawn_params);
				p->position.xy += spawn_position;
				remaining_particles_to_spawn -= 1;
			}
		}
		
		next_explosion_interval = random_get1(explosion_interval);
		explosion_accumulation_timer = 0;
	}
	
	for (int i = 0; i < system->count; i += 1) {
		Particle* p = &system->particles[i];
		
		p->velocity.xy += gravity * dt;
		p->velocity *= friction;
		p->position += p->velocity * dt;
		p->life -= dt;
		p->color.w = fmin(p->color.w, p->life);
		
		if (p->life < 0) p->scale = 0;
	}
	
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE); // #temporary
	render_target_clear(hdr_render_target, {0, 0, 0, 1});
	particle_system_upload_and_render(system, circle_mesh, &particles_render_state);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // #temporary: Reset default opengl state

	hdr_blit_render_state.texture0 = render_target_flush(hdr_render_target);
	mesh_render(square_mesh, &hdr_blit_render_state);
	
	ImGui::RangeEdit("Number of particles", &particles_per_explosion, 0, 1000);
	ImGui::RangeEdit("Interval", &explosion_interval, 1, 10);
	ImGui::RangeEdit("Color", &spawn_params.color);
	ImGui::RangeEdit("Life", &spawn_params.life, 0, 5);
	ImGui::RangeEdit("Size", &spawn_params.scale, 0, 1);
	
	ImGui::Separator();
	
	ImGui::InputFloat("Gravity", &gravity.y, -1, 1);
	ImGui::SliderFloat("Friction", &friction, 0, 1);
	
}