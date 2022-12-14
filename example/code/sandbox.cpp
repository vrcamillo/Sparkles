#include "sandbox.h"

Mesh* mesh_presets[NUM_MESH_PRESETS];
Texture* texture_presets[NUM_TEXTURE_PRESETS];

// 
// Graphics variables.
//
RenderTarget* hdr_render_target;
RenderState hdr_blit_render_state;

//
// Particle simulation variables.
//

SandboxState state;

// We do not store our system in the Emitter struct, so we create our simulation variables for each emitter as an external variable.
ParticleSystem* systems[max_emitter_count];
float emission_accumulation_timer[max_emitter_count];
float next_emission_interval[max_emitter_count];

void sandbox_ui(SandboxState* state, float dt);

bool sandbox_state_load(SandboxState* state, const char* file_path) {
	bool result = false;
	
	auto file = fopen(file_path, "rb");
	if (file) {
		fseek(file, 0, SEEK_END);
		auto size = ftell(file);
		fseek(file, 0, SEEK_SET);
		
		if (size == sizeof(SandboxState)) {
			fread(state, sizeof(SandboxState), 1, file);
			result = true;
		}
		fclose(file);
	}
	
	return result;
}

void sandbox_state_save(SandboxState* state, const char* file_path) {	
	auto file = fopen(file_path, "wb");
	if (file) {
		fwrite(state, sizeof(SandboxState), 1, file);		
		fclose(file);
	}
}


bool sandbox_init() {
	// Before creating any variable, we must initialize Sparkles. 
	// (Maybe this we should get rid of this step to simplify the #api?)
	
	Sparkles::initialize();
	
	{
		//
		// Initialize our graphics variables.
		//
		
		immediate_init();
		
		// Create our HDR render target the same size as our backbuffer.
		int backbuffer_width, backbuffer_height;
		glfwGetFramebufferSize(the_window, &backbuffer_width, &backbuffer_height);
		
		hdr_render_target = render_target_create(TextureFormat::RGBA_FLOAT16, backbuffer_width, backbuffer_height);
		
		hdr_blit_render_state.projection = orthographic(0, 1, 1, 0, -1, +1);
		hdr_blit_render_state.viewport = {0, 0, (float) backbuffer_width, (float) backbuffer_height};
		hdr_blit_render_state.texture0 = nullptr; // Will be our HDR texture.
	}
	
	{
		// Create our mesh presets.
		immediate_rect({-0.5f, -0.5f, 1.0f, 1.0f});
		auto square = immediate_mesh();
		
		int circle_sides = 20; // For our circle, we create a regular polygon of 20 sides.
		immediate_regular_polygon({0, 0}, 0.5f, 20); 
		auto circle = immediate_mesh();
		
		mesh_presets[0] = square;
		mesh_presets[1] = circle;
	}
	
	{
		// Create our texture presets.
		texture_presets[0] = nullptr;
		texture_presets[1] = texture_generate_light_mask(64, 64, 0.01, 1);
		texture_presets[2] = texture_generate_light_mask(64, 64, 0.01, 5);
		texture_presets[3] = texture_generate_light_mask(64, 64, 0.01, 10);
	}
	
	{
		// Initialize the sandbox and particle states.
		
		sandbox_state_init(&state);
		
		for (int e = 0; e < max_emitter_count; e += 1) {		
			auto system = particle_system_create(max_particles_per_emitter);
			
			for (int i = 0; i < system->count; i += 1) {
				Particle* p = &system->particles[i];
				p->scale = 0;
				p->life = -1;
			}
			
			systems[e] = system;
		}
	}
	
	{
		// Load default sample.
		sandbox_state_load(&state, "default.sparkles");
	}
	
	return true;
}

void notify_starvation(int count);

void sandbox_frame(float dt) {
	render_target_clear(nullptr, {0, 0, 0, 1});	
		
	RenderState render_state;
	render_state.render_target = hdr_render_target;
	render_state.projection = orthographic(-state.space_width * 0.5, +state.space_width * 0.5, +state.space_height * 0.5, -state.space_height * 0.5, -1, +1);
	render_state.viewport = {0, 0, (float) hdr_render_target->width, (float) hdr_render_target->height};
	
	render_target_clear(hdr_render_target, {0, 0, 0, 1});
	
	Physics physics = state.physics;
	
	// For each emitter, spawn new particles, if it is time to do so.
	for (int s = 0; s < state.emitter_count; s += 1) {
		auto system = systems[s];
		auto emitter = &state.emitters[s];
		if (!emitter->active) continue;
		
		emission_accumulation_timer[s] += dt;
		if (next_emission_interval[s] < 0 || emission_accumulation_timer[s] >= next_emission_interval[s]) {
			int remaining_particles_to_spawn = random_get1(emitter->particles_per_emission);
			
			for (int i = 0; i < system->count && remaining_particles_to_spawn > 0; i += 1) {
				Particle* p = &system->particles[i];
				
				if (p->life < 0) { // Spawn particles in the place of those that have already died.
					
					p->position.xy = emitter->position + random_get2(emitter->offset);
					p->velocity.xy = random_get2(emitter->velocity);
					p->life = random_get1(emitter->life);
					p->scale = random_get1(emitter->size);
					
					{
						// Choose color based on the color weights
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
		
		// Here is our simple simulation loop.
		for (int i = 0; i < system->count; i += 1) {
			Particle* p = &system->particles[i];
			
			if (p->life < 0) continue; // Do not simulate particles that have already died.
			
			// Apply gravity.
			p->velocity.xy += physics.gravity * dt;
			
			// Apply force fields.
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
			
			// Integrate our position.
			p->position += p->velocity * dt;
			
			// Apply friction.
			p->velocity *= physics.friction;
			
			// Decrease the particle's life.
			p->life -= dt;
			
			// Make the particles fade as they die. (A bit of a #hardcoded effect).
			p->color.w = fmin(p->color.w, p->life);
			
			// If the particle is dead, set its size to 0, so that it is never rendered.
			if (p->life < 0) p->scale = 0;
		}
		
		render_state.texture0 = texture_presets[emitter->texture_index];
		
		if (emitter->texture_index > 0) glBlendFunc(GL_SRC_ALPHA, GL_ONE); // #temporary
		particle_system_upload_and_render(system, mesh_presets[emitter->mesh_index], &render_state);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // #temporary: Reset default opengl state
	}
	
	hdr_blit_render_state.texture0 = render_target_flush(hdr_render_target);
	
	immediate_rect({0, 0, 1, 1});
	immediate_flush(&hdr_blit_render_state);
	
	sandbox_ui(&state, dt);
}

void sandbox_state_init(SandboxState* state) {
	memset(state, 0, sizeof(SandboxState));
	
	state->version = serialization_version;
	
	state->space_width = 16; // #hardcoded
	state->space_height = 9; // #hardcoded
	
	physics_init(&state->physics);
	
	state->emitter_count = 1;
	emitter_init(&state->emitters[0]);
}

void physics_init(Physics* physics) {
	memset(physics, 0, sizeof(*physics));
	physics->gravity = polar(0.001, -TAU * 0.25);	
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
