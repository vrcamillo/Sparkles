#include "common.h"

#include "sparkles.h"
#include "sparkles_utils.h"
using namespace Sparkles;

//
// Particle system parameters 
//
ParticleSystem* system;
constexpr int max_particle_count = 5000;

struct Params {
	vec2 explosion_position = {0, +1};
	Range1 explosion_interval = {1, 2};
	Range1 particles_per_explosion = {300, 700};
	
	vec2 gravity = {0, -4};
	float friction = 0.98;
	
	Range2 spawn_offset = polar(0, 0, 0, TAU);
	Range2 spawn_velocity = polar(0, 8, 0, TAU);
	Range1 size = {0.3, 0.7};
	Range1 life = {0.5, 2};
	
	int color_count = 2;
	vec4 colors[8] = {
		{0.8, 0.2, 0.3, 1},
		{1, 0.5, 0.6, 1},
	};
};

Params params;

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
		light_mask = texture_generate_light_mask(64, 64, 0.01, 10);
		
		particles_render_state.render_target = hdr_render_target;
		particles_render_state.projection = orthographic(-8, +8, +5, -5, -1, +1);
		particles_render_state.viewport = {0, 0, (float) hdr_render_target->width, (float) hdr_render_target->height};
		particles_render_state.texture0 = light_mask;
		
		hdr_blit_render_state.projection = orthographic(-0.5f, +0.5f, +0.5f, -0.5f, -1, +1);;
		hdr_blit_render_state.viewport = {0, 0, (float) backbuffer_width, (float) backbuffer_height};
		hdr_blit_render_state.texture0 = nullptr; // Will be our HDR texture.
		
	}
	
}

void firework_panel() {
	
	using namespace ImGui;
	
	SetNextWindowPos(ImVec2(0, 0));
	Begin("Fireworks!", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	
	DragFloatRange2("Particles per explosion", &params.particles_per_explosion.min, &params.particles_per_explosion.max, 5, 0, 1000, "Min: %.0f", "Max: %.0f", ImGuiSliderFlags_AlwaysClamp);
	DragFloatRange2("Explosion interval", &params.explosion_interval.min, &params.explosion_interval.max, 0.05f, 0, 10, "Min: %.1f s", "Max: %.1f s", ImGuiSliderFlags_AlwaysClamp);
	
	Separator();
	
	Text("Particle");
	
	DragFloatRange2("Speed", &params.spawn_velocity.min.x, &params.spawn_velocity.max.x, 0.1, 0, 20, "Min: %.1f m/s", "Max: %.1f m/s", ImGuiSliderFlags_AlwaysClamp);
	DragFloatRange2("Life", &params.life.min, &params.life.max, 0.05, 0, 5, "Min: %.1f s", "Max: %.1f s", ImGuiSliderFlags_AlwaysClamp);
	DragFloatRange2("Size", &params.size.min, &params.size.max, 0.01, 0, 2, "Min: %.2f m", "Max: %.2f m", ImGuiSliderFlags_AlwaysClamp);
	
	if (ImGui::TreeNode("Colors")) {
		int delete_index = -1;
		for (int i = 0; i < params.color_count; i += 1) {
			PushID(i);
						
			ColorEdit4("##palette", (float*) &params.colors[i], ImGuiColorEditFlags_NoAlpha/* | ImGuiColorEditFlags_NoInputs*/);
			if (i != 0) {
				SameLine();
				if (Button("-", ImVec2(20, 20))) delete_index = i;
			}

			PopID();
		}
		
		if (params.color_count < array_size(params.colors)) {
			if (Button("+", ImVec2(20, 20))) {
				params.colors[params.color_count] = params.colors[params.color_count - 1];
				params.color_count += 1;
			}
		}
		
		if (delete_index >= 0) {
			for (int i = delete_index; i < params.color_count - 1; i += 1) {
				params.colors[i] = params.colors[i + 1];
			}
			params.color_count -= 1;
		}
		
		ImGui::TreePop();
	}
	
	
	// RangeEdit("Color", &params.spawn.color);
	
	Separator();
	
	Text("Physics");
	
	SliderFloat("Gravity", &params.gravity.y, -10, 10);
	SliderFloat("Friction", &params.friction, 0.9, 1, "%.3f", ImGuiSliderFlags_Logarithmic);
	
	End();
}

void firework_frame(float dt) {
	firework_panel();
	
	explosion_accumulation_timer += dt;
	
	if (next_explosion_interval < 0 || explosion_accumulation_timer > next_explosion_interval) {		
		int remaining_particles_to_spawn = (int) random_get1(params.particles_per_explosion);
		
		for (int i = 0; i < system->count && remaining_particles_to_spawn > 0; i += 1) {
			Particle* p = &system->particles[i];
			
			if (p->life < 0) {
				p->position.xy = params.explosion_position + random_get2(params.spawn_offset);
				p->velocity.xy = random_get2(params.spawn_velocity);
				p->life = random_get1(params.life);
				p->scale = random_get1(params.size);
				
				float color_number = params.color_count * random_get();
				int color0_index = fmax((int) (color_number - 0.5), 0);
				int color1_index = fmin((int) (color_number + 0.5), params.color_count - 1);
				
				float t = random_get();
				p->color = (1 - t) * params.colors[color0_index] + t * params.colors[color1_index];
				
				remaining_particles_to_spawn -= 1;
			}
		}
		
		next_explosion_interval = random_get1(params.explosion_interval);
		explosion_accumulation_timer = 0;
	}
	
	for (int i = 0; i < system->count; i += 1) {
		Particle* p = &system->particles[i];
		
		p->velocity.xy += params.gravity * dt;
		p->velocity *= params.friction;
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
}
