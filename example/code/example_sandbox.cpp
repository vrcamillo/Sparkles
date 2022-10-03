
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
	float color_weights[16];
};

enum class ForceType {
	LINEAR,
	INVERSE,
	INVERSE_SQUARED,
};

static const char* force_type_names[] = {
	"Linear (kr)",
	"Inverse (k/r)",
	"Inverse squared (k/r^2)",
};

struct Attractor {
	bool active;
	
	vec2 position;
	ForceType force_type;
	float radius;
	float factor;
	float magnitude_cap;
};

struct Physics {
	vec2 gravity;
	float friction;
	
	int attractor_count;
	Attractor attractors[8];
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
	emitter->color_weights[0] = 1;
}

void physics_init(Physics* physics) {
	physics->gravity = {0, 0};
	physics->friction = 1;
}

void attractor_init(Attractor* attractor) {
	memset(attractor, 0, sizeof(Attractor));
	attractor->active = true;
	attractor->position = {0, 0};
	attractor->radius = 0.5;
	attractor->factor = 1;	
	attractor->force_type = ForceType::LINEAR;
	attractor->magnitude_cap = 10;
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


template <typename T>
void array_ordered_remove(int* array_count, T array[], int index_to_remove) {
	for (int i = index_to_remove; i < *array_count - 1; i += 1) {
		array[i] = array[i + 1];
	}
	*array_count -= 1;
}

constexpr ImVec4 add_color = ImVec4(0.1, 0.5, 0.1, 1);
constexpr ImVec4 delete_color = ImVec4(0.8, 0.1, 0.1, 1);

bool ColoredButton(ImVec4 color, const char* label) {
	ImGui::PushStyleColor(ImGuiCol_Button, color);
	bool result = ImGui::Button(label);
	ImGui::PopStyleColor();
	return result;
}

bool debug_view;

void sandbox_panel() {
	using namespace ImGui;
	
	SetNextWindowPos(ImVec2(0, 0));
	Begin("Sandbox!", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	Checkbox("Debug visualization", &debug_view);
	
	if (TreeNodeEx("Physics", ImGuiTreeNodeFlags_DefaultOpen)) {
		Physics* physics = &state.physics;
		
		static float gravity_angle = TAU * 0.75f;
		static float gravity_magnitude = 1;
		SliderAngle("Gravity angle", &gravity_angle, 0, 360);
		SameLine(); if (Button("^")) gravity_angle = TAU * 0.25f;
		SameLine(); if (Button("v")) gravity_angle = TAU * 0.75f;
		DragFloat("Gravity magnitude", &gravity_magnitude, 0.1, 0, 20, "%.1f"); 		
		physics->gravity = polar(gravity_magnitude, gravity_angle);
		
		SliderFloat("Friction factor", &physics->friction, 0.8, 1, "%.3f", ImGuiSliderFlags_Logarithmic);
		
		int delete_attractor_index = -1;
		for (int i = 0; i < physics->attractor_count; i += 1) {
			Attractor* attractor = &physics->attractors[i];
			
			PushID(i);
			
			if (TreeNodeEx("Attractor", ImGuiTreeNodeFlags_DefaultOpen, "Attractor %d", i + 1)) {
				
				Checkbox("Active", &attractor->active);
				SameLine();
				if (ColoredButton(delete_color, "Delete")) delete_attractor_index = i;
			
				DragFloat2("Position (x, y)", &attractor->position.x, 0.05, -10, +10, "%.1f");
				DragFloat("Influence radius", &attractor->radius, 0.05, 0, +20, "%.1f");
				
				Combo("Force type", (int*) &attractor->force_type, force_type_names, array_size(force_type_names));
				DragFloat("Factor", &attractor->factor, 0.1, -100, +100, "%.1f");
				DragFloat("Magnitude cap", &attractor->magnitude_cap, 0.1, 0, 100, "%.1f");
				TreePop();
			}
			PopID();
		}
		
		if (physics->attractor_count < array_size(physics->attractors)) {

			if (ColoredButton(add_color, "+ New attractor")) {
				if (physics->attractor_count > 0) {
					physics->attractors[physics->attractor_count] = physics->attractors[physics->attractor_count - 1];
				} else {
					attractor_init(&physics->attractors[physics->attractor_count]);
				}
				physics->attractor_count += 1;
			}
		}
		
		if (delete_attractor_index >= 0) array_ordered_remove(&physics->attractor_count, physics->attractors, delete_attractor_index);
		
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
				if (ColoredButton(delete_color, "Delete")) delete_emitter_index = s;
			}
			
			ImGui::BulletText("Simulation");
			
			DragFloat2("Position (x, y)", &emitter->position.x, 0.05, -10, +10, "%.1f");
			
			DragFloatRange2("Emission period", &emitter->emission_interval.min, &emitter->emission_interval.max, 0.01, 0.05, 10, "min = %.2f s", "max = %.2f s", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_Logarithmic);
			DragFloatRange2("Particles per emission", &emitter->particles_per_emission.min, &emitter->particles_per_emission.max, 0.5, 0, system_particle_count, "min = %.0f", "max = %.0f", ImGuiSliderFlags_AlwaysClamp);
			
			ImGui::BulletText("Visuals");
			
			Combo("Shape", &emitter->mesh_index, mesh_presets_names, NUM_MESH_PRESETS); 
			Combo("Texture", &emitter->texture_index, texture_presets_names, NUM_TEXTURE_PRESETS); 
			
			int delete_color_index = -1;
			for (int i = 0; i < emitter->color_count; i += 1) {
				PushID(i);
				
				ColorEdit4("##palette", (float*) &emitter->colors[i], ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoInputs);
				SameLine();
				Text("Color %d", i + 1);
				
				SameLine();
				SetNextItemWidth(100); 
				DragFloat("", &emitter->color_weights[i], 0.05, 0, 5, "weight = %.1f");
				
				if (i != 0) {
					SameLine();
					if (ColoredButton(delete_color, " - ")) delete_color_index = i;
				}
				
				PopID();
			}
			
			if (emitter->color_count < array_size(emitter->colors)) {
				PushStyleColor(ImGuiCol_Button, add_color);
				if (Button("+ New color")) {
					emitter->colors[emitter->color_count] = emitter->colors[emitter->color_count - 1];
					emitter->color_weights[emitter->color_count] = 1;
					emitter->color_count += 1;
				}
				PopStyleColor();
			}
			
			if (delete_color_index >= 0) {
				array_ordered_remove(&emitter->color_count, emitter->colors, delete_color_index);	
				emitter->color_count += 1; // #hack
				array_ordered_remove(&emitter->color_count, emitter->color_weights, delete_color_index);
			}
			
			ImGui::BulletText("Particle");
			
			static_assert(sizeof(emitter->offset.coords) == sizeof(int));
			Text("Offset coords: ");
			SameLine();
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
	
	if (state.emitter_count < max_system_count) {
		if (ColoredButton(add_color, "+ New emitter")) {
			state.emitters[state.emitter_count] = state.emitters[state.emitter_count - 1];
			state.emitter_count += 1;
		}
	}
	
	if (delete_emitter_index >= 0) array_ordered_remove(&state.emitter_count, state.emitters, delete_emitter_index);
	
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
					
					{
						// Choose color based on weights
						float total = 0;
						for (int i = 0; i < emitter->color_count; i += 1) total += emitter->color_weights[i];
						
						vec4 chosen_color = {};
						
						float random_number = total * random_get();
						float cursor = 0;
						for (int i = 0; i < emitter->color_count; i += 1) {
							float weight = emitter->color_weights[i];
							if (random_number < cursor + weight) {
								chosen_color = emitter->colors[i];
								break;
							}
							cursor += weight;
						}
						
						p->color = chosen_color;
					}
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
			
			// #speed: This is not the most optimal thing. 
			for (int a = 0; a < physics.attractor_count; a += 1) {
				Attractor attractor = physics.attractors[a];
				if (!attractor.active) continue;
				
				vec2 delta = attractor.position - p->position.xy; // Points to attractor
				float distance2 = norm2(delta);
				float distance = sqrt(distance2);
				vec2 direction = normalize(delta);
				
				vec2 force = {0, 0};
				if (distance2 < attractor.radius * attractor.radius) {
					switch (attractor.force_type) {
					  case ForceType::INVERSE_SQUARED: force = (attractor.factor / distance2) * direction; break;
					  case ForceType::INVERSE: force = (attractor.factor / distance) * direction; break;
					  case ForceType::LINEAR: force = (attractor.factor * distance) * direction; break; 
					}
				}
				
				auto force_magnitude = norm(force);
				if (force_magnitude > attractor.magnitude_cap) force *= attractor.magnitude_cap / force_magnitude;
				
				p->velocity.xy += force * dt;
			}
			
			p->position += p->velocity * dt;
			p->velocity *= physics.friction;
			
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