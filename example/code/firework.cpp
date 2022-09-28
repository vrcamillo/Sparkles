#include "common.h"

#include "sparkles.h"
#include "sparkles_utils.h"
using namespace Sparkles;

static ParticleSystem* system;

static const char* hdr_pixel_shader_source = R"glsl(
#version 330 core

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec4 vertex_color;
layout (location = 2) in vec2 vertex_uv;

layout (location = 3) in vec3 instance_position;
layout (location = 4) in float instance_scale;
layout (location = 5) in vec4 instance_color;

uniform mat4 projection;

out vec4 pixel_color;

void main() {
	vec4 world_position = vec4(instance_position + vertex_position * instance_scale, 1);
	gl_Position = projection * world_position;
	pixel_color = vertex_color * instance_color;
}  
)glsl";

static RandomVec2 explosion_position = {
	.coordinate_system = CoordinateSystem::RECTANGULAR,
	.x = {Distribution::UNIFORM, -0.5, +0.5},
	.y = {Distribution::UNIFORM, -0.5, +0.5},
};

static RandomScalar explosion_interval = {Distribution::UNIFORM, 0.25, 1};
static RandomScalar particles_per_explosion = {Distribution::UNIFORM, 50, 100};

float explosion_accumulation_timer;
float next_explosion_interval;

static PhysicsParams physics = {
	.gravity = {0, -1},
	.friction = 0.99,
};

static SpawnParams spawn = {
	.position = {
		.coordinate_system = CoordinateSystem::POLAR,
		.angle = {Distribution::UNIFORM, 0, PI * 2.0f},
		.radius = {Distribution::UNIFORM, 0, 0.1f},
	},
	
	.scale = {Distribution::UNIFORM, 0.001, 0.05},
	
	.color = {
		.color_system = ColorSystem::RGB,
		.red   = {Distribution::UNIFORM, 0.8, 1},
		.green = {Distribution::UNIFORM, 0.3, 1},
		.blue  = {Distribution::UNIFORM, 0.3, 1},
		.alpha = {Distribution::UNIFORM, 0.8, 1},
	},
	
	.velocity = {
		.coordinate_system = CoordinateSystem::POLAR,
		.angle = {Distribution::UNIFORM, 0, PI * 2.0f},
		.radius = {Distribution::UNIFORM, 4, 5},
	},
	
	.life = {Distribution::UNIFORM, 0.5, 2},
};

RenderTarget* hdr_render_target;
int render_target_width;
int render_target_height;

Shader* hdr_pixel_shader;

void firework_init() {	
	system = particle_system_create(1000);
	
	for (int i = 0; i < system->count; i += 1) {
		Particle* p = &system->particles[i];
		particle_spawn(p, &spawn);
		p->scale = 0;
	}
	
	next_explosion_interval = random_get(explosion_interval);
	
	// Create our hdr render target the same size as our backbuffer.
	glfwGetFramebufferSize(global_window, &render_target_width, &render_target_height);
	hdr_render_target = render_target_create(TextureFormat::RGBA_FLOAT16, render_target_width, render_target_height);
	
	hdr_pixel_shader = shader_create(ShaderLanguage::GLSL, ShaderType::PIXEL, hdr_pixel_shader_source);
	
}

void firework_frame(float dt) {
	explosion_accumulation_timer += dt;
	
	if (explosion_accumulation_timer > next_explosion_interval) {
		vec2 spawn_position = random_get(explosion_position);
		int remaining_particles_to_spawn = (int) random_get(particles_per_explosion);
		
		for (int i = 0; i < system->count && remaining_particles_to_spawn > 0; i += 1) {
			Particle* p = &system->particles[i];
			if (p->life < 0) {
				particle_spawn(p, &spawn);
				p->position.xy += spawn_position;
				remaining_particles_to_spawn -= 1;
			}
		}
		
		next_explosion_interval = random_get(explosion_interval);
		explosion_accumulation_timer = 0;
	}
	
	for (int i = 0; i < system->count; i += 1) {
		Particle* p = &system->particles[i];
		particle_simulate(p, &physics, dt);
		
		if (p->life < 0) p->scale = 0;
		
		p->color.w *= 0.95;
	}
	
	RenderState render_state;
	render_state.render_target = hdr_render_target;
	render_state.pixel_shader = hdr_pixel_shader;
	render_state.projection = orthographic(-0.8f, +0.8f, +0.5f, -0.5f, -1, +1);
	render_state.viewport = {0, 0, (float) render_target_width, (float) render_target_height};
	
	particle_system_upload_and_render(system, circle_mesh, &render_state);
	
	render_state.render_target = nullptr;
	render_state.projection = mat4_identity();
	render_state.viewport = {0, 0, (float) render_target_width, (float) render_target_height};
	
	mesh_render(square_mesh, &render_state);
	
}