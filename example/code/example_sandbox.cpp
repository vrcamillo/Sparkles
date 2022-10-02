
#include "common.h"

#include "sparkles.h"
#include "sparkles_utils.h"
using namespace Sparkles;

constexpr int system_particle_count = 5000;
constexpr int max_system_count = 8;

static ParticleSystem* systems[max_system_count];

//
// Particle system parameters 
//

struct Emitter {
	bool active;
	
	vec2 position;
	
	Range1 emission_interval;
	Range1 particles_per_emission;
	
	int mesh_index;
	int texture_index;
	
	Range2 offset;
	Range2 velocity;
	Range1 size;
	Range1 life;
	
	int color_count;
	vec4 colors[16];
};

struct Attractor {
	vec2 position;
	float radius;
	float magnitude;
};

struct Physics {
	vec2 gravity;
	float friction;
	
	int attractor_count;
	Attractor attractors[16];
};

struct SandboxState {
	int version;
	
	float space_width;
	float space_height;
		
	Physics physics;
	
	int emitter_count;
	Emitter emitters[max_system_count];
};

SandboxState state;

//
// Simulation variables
//
float emission_accumulation_timer[max_system_count];
float next_emission_interval[max_system_count];

//
// Graphics variables
// 


//
// Initialization functions
//
void emitter_init(Emitter* emitter) {
	memset(emitter, 0, sizeof(Emitter));
	
	emitter->active = true;
	
	emitter->emission_interval = {1, 1};
	emitter->particles_per_emission = {0, 10};
	
	emitter->velocity = range_polar(0, 1, 0, TAU);
	emitter->size = {0.5, 0.5};
	emitter->life = {1, 1};
	
	emitter->color_count = 1;
	emitter->colors[0] = {1, 1, 1, 1};
}

void physics_init(Physics* physics) {
	physics->gravity = {0, 0};
	physics->friction = 1;
}

void sandbox_state_init(SandboxState* state) {
	memset(state, 0, sizeof(SandboxState));
	
	state->version = 1;
	
	state->space_width = 16;
	state->space_height = 9;
	
	physics_init(&state->physics);
	
	state->emitter_count = 1;
	emitter_init(&state->emitters[0]);
}

//
//
//

void sandbox_init() {
	sandbox_state_init(&state);
	
	for (int s = 0; s < max_system_count; s += 1) {		
		auto system = particle_system_create(system_particle_count);
		
		// Initialize all particles.
		for (int i = 0; i < system->count; i += 1) {
			Particle* p = &system->particles[i];
			p->scale = 0;
			p->life = -1;
		}
		
		systems[s] = system;
	}
}


void DragAngleRange2(const char* label, float* min_radians, float* max_radians, float speed = 1, float min_degrees = 0, float max_degrees = 360) {
	float min_value_degrees = *min_radians * (360.0f / TAU);
	float max_value_degrees = *max_radians * (360.0f / TAU);
	ImGui::DragFloatRange2(label, &min_value_degrees, &max_value_degrees, speed, min_degrees, max_degrees, "Min: %.0f deg", "Max: %.0f deg", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SameLine(); if (ImGui::Button("All")) { min_value_degrees = 0; max_value_degrees = 360;}
	*min_radians = min_value_degrees * (TAU / 360.0);
	*max_radians = max_value_degrees * (TAU / 360.0);
}

void sandbox_panel() {
	using namespace ImGui;
	
	SetNextWindowPos(ImVec2(0, 0));
	Begin("Sandbox!", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	
	#if 0
	if (TreeNodeEx("Globals", ImGuiTreeNodeFlags_DefaultOpen)) {
		DragFloat("Space width", &state.space_width, 0.5, 1, 100); 
		DragFloat("Space height", &state.space_height, 0.5, 1, 100); 
		TreePop();
	}
	#endif

	
	if (TreeNodeEx("Physics", ImGuiTreeNodeFlags_DefaultOpen)) {
		static float gravity_angle = TAU * 0.75f;
		static float gravity_magnitude = 1;
		SliderAngle("Gravity angle", &gravity_angle, 0, 360);
		SameLine(); if (Button("^")) gravity_angle = TAU * 0.25f;
		SameLine(); if (Button("v")) gravity_angle = TAU * 0.75f;
		SliderFloat("Gravity magnitude", &gravity_magnitude, 0, 20, "%.1f"); 		
		state.physics.gravity = polar(gravity_magnitude, gravity_angle);
		
		SliderFloat("Friction factor", &state.physics.friction, 0.8, 1, "%.3f", ImGuiSliderFlags_Logarithmic);
		
		TreePop();
	}
	
	int delete_emitter_index = -1;
	for (int s = 0; s < state.emitter_count; s += 1) {
		auto emitter = &state.emitters[s];
		
		PushID(s);
		
		if (TreeNodeEx("Emitter", ImGuiTreeNodeFlags_DefaultOpen, "Emitter %d", s + 1)) {
			Checkbox("Active", &emitter->active);
			
			if (s != 0) {
				SameLine();
				PushStyleColor(ImGuiCol_Button, (ImVec4)ImVec4(0.8, 0.1, 0.1, 1));
				if (Button("Delete")) delete_emitter_index = s;
				PopStyleColor();
			}
			
			Separator();
			ImGui::BulletText("Simulation");
			
			DragFloat2("Position (x, y)", &emitter->position.x, 0.05, -10, +10, "%.1f");
			
			DragFloatRange2("Emission period", &emitter->emission_interval.min, &emitter->emission_interval.max, 0.01, 0.05, 10, "min = %.2f s", "max = %.2f s", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_Logarithmic);
			DragFloatRange2("Particles per emission", &emitter->particles_per_emission.min, &emitter->particles_per_emission.max, 0.5, 0, system_particle_count, "min = %.0f", "max = %.0f", ImGuiSliderFlags_AlwaysClamp);
			
			Separator();
			ImGui::BulletText("Visuals");
			
			Combo("Shape", &emitter->mesh_index, mesh_presets_names, NUM_MESH_PRESETS); 
			Combo("Texture", &emitter->texture_index, texture_presets_names, NUM_TEXTURE_PRESETS); 
			
			int delete_color_index = -1;
			for (int i = 0; i < emitter->color_count; i += 1) {
				PushID(i);
				
				ColorEdit4("##palette", (float*) &emitter->colors[i], ImGuiColorEditFlags_Float/* | ImGuiColorEditFlags_NoInputs*/);
				SameLine();
				Text("Color %d", i + 1);
				if (i != 0) {
					SameLine();
					if (Button(" - ")) delete_color_index = i;
				}
				
				PopID();
			}
			
			if (emitter->color_count < array_size(emitter->colors) && Button("+ New color")) {
				emitter->colors[emitter->color_count] = emitter->colors[emitter->color_count - 1];
				emitter->color_count += 1;
			}
			
			if (delete_color_index >= 0) {
				for (int i = delete_color_index; i < emitter->color_count - 1; i += 1) {
					emitter->colors[i] = emitter->colors[i + 1];
				}
				emitter->color_count -= 1;
			}
			
			
			Separator();
			ImGui::BulletText("Particle");
			
			static_assert(sizeof(emitter->offset.coords) == sizeof(int));
			RadioButton("Cartesian", (int*) &emitter->offset.coords, (int) Coords2D::CARTESIAN);
			SameLine();
			RadioButton("Polar", (int*) &emitter->offset.coords, (int) Coords2D::POLAR);
			
			switch (emitter->offset.coords) {
			  case Coords2D::CARTESIAN: 
				DragFloatRange2("X offset", &emitter->offset.min.x, &emitter->offset.max.x, 0.1, -10, +10, "Min: %.1f", "Max: %.1f", ImGuiSliderFlags_AlwaysClamp);
				DragFloatRange2("Y offset", &emitter->offset.min.y, &emitter->offset.max.y, 0.1, -10, +10, "Min: %.1f", "Max: %.1f", ImGuiSliderFlags_AlwaysClamp);
				break;
			  case Coords2D::POLAR:
				DragFloatRange2("Radius offset", &emitter->offset.min.x, &emitter->offset.max.x, 0.1, 0, 10, "Min: %.1f", "Max: %.1f", ImGuiSliderFlags_AlwaysClamp);
				DragAngleRange2("Angle offset", &emitter->offset.min.y, &emitter->offset.max.y);
				
			  default: break;
			}
			
			DragFloatRange2("Speed", &emitter->velocity.min.x, &emitter->velocity.max.x, 0.1, 0, 20, "Min: %.1f", "Max: %.1f", ImGuiSliderFlags_AlwaysClamp);
			DragAngleRange2("Direction", &emitter->velocity.min.y, &emitter->velocity.max.y);
			DragFloatRange2("Size", &emitter->size.min, &emitter->size.max, 0.01, 0, 4, "min = %.2f", "max = %.2f", ImGuiSliderFlags_AlwaysClamp);
			DragFloatRange2("Life", &emitter->life.min, &emitter->life.max, 0.05, 0, 20, "min = %.1f s", "max = %.1f s", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_Logarithmic);
			
			
			TreePop();	
		}
		
		PopID();
	}
	
	if (state.emitter_count < max_system_count && Button("+ New emitter")) {
		state.emitters[state.emitter_count] = state.emitters[state.emitter_count - 1];
		state.emitter_count += 1;
	}
	
	if (delete_emitter_index >= 0) {
		for (int i = delete_emitter_index; i < state.emitter_count - 1; i += 1) {
			state.emitters[i] = state.emitters[i + 1];
		}
		state.emitter_count -= 1;
	}
	
	End();
}


void sandbox_frame(float dt) {
	sandbox_panel();
	
	RenderState render_state;
	render_state.render_target = hdr_render_target;
	render_state.projection = orthographic(-state.space_width * 0.5, +state.space_width * 0.5, +state.space_height * 0.5, -state.space_height * 0.5, -1, +1);
	render_state.viewport = {0, 0, (float) hdr_render_target->width, (float) hdr_render_target->height};
	
	
	render_target_clear(hdr_render_target, {0, 0, 0, 1});
	
	Physics physics = state.physics;
	for (int s = 0; s < state.emitter_count; s += 1) {
		auto system = systems[s];
		auto emitter = &state.emitters[s];
		if (!emitter->active) continue;
		
		emission_accumulation_timer[s] += dt;
		if (next_emission_interval[s] < 0 || emission_accumulation_timer[s] >= next_emission_interval[s]) {
			int remaining_particles_to_spawn = random_get1(emitter->particles_per_emission);
			
			for (int i = 0; i < system->count && remaining_particles_to_spawn > 0; i += 1) {
				Particle* p = &system->particles[i];
				
				if (p->life < 0) {
					p->position.xy = emitter->position + random_get2(emitter->offset);
					p->velocity.xy = random_get2(emitter->velocity);
					p->life = random_get1(emitter->life);
					p->scale = random_get1(emitter->size);
					
					float color_number = emitter->color_count * random_get();
					int color0_index = fmax((int) (color_number - 0.5), 0);
					int color1_index = fmin((int) (color_number + 0.5), emitter->color_count - 1);
					
					float t = random_get();
					p->color = (1 - t) * emitter->colors[color0_index] + t * emitter->colors[color1_index];
					
					remaining_particles_to_spawn -= 1;
				}
			}
			
			notify_starvation(remaining_particles_to_spawn);
			
			next_emission_interval[s] = random_get1(emitter->emission_interval);
			emission_accumulation_timer[s] = 0;
		}
		
		for (int i = 0; i < system->count; i += 1) {
			Particle* p = &system->particles[i];
			
			p->velocity.xy += physics.gravity * dt;
			p->velocity *= physics.friction;
			p->position += p->velocity * dt;
			p->life -= dt;
			p->color.w = fmin(p->color.w, p->life);
			
			if (p->life < 0) p->scale = 0;
		}
		
		render_state.texture0 = texture_presets[emitter->texture_index];
		
		if (emitter->texture_index > 0) glBlendFunc(GL_SRC_ALPHA, GL_ONE); // #temporary
		particle_system_upload_and_render(system, mesh_presets[emitter->mesh_index], &render_state);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // #temporary: Reset default opengl state
		
	}
}