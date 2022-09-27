#include "common.h"

using namespace Sparkles;

ParticleSystem* the_system;

Mesh* square_mesh;
Mesh* circle_mesh;

mat4 projection_matrix;

bool initialize() {
	{
		// Create mesh presets.
		
		{
			// Square 
			
			Vertex vertices[4] = {
				{.position = {-0.5f, -0.5f, 0}, .color = {1, 1, 1, 1}, .uv = {0, 0}},
				{.position = {+0.5f, -0.5f, 0}, .color = {1, 1, 1, 1}, .uv = {1, 0}},
				{.position = {+0.5f, +0.5f, 0}, .color = {1, 1, 1, 1}, .uv = {1, 1}},
				{.position = {-0.5f, +0.5f, 0}, .color = {1, 1, 1, 1}, .uv = {0, 1}},
			};
			
			uint32_t indices[6] = {
				0, 1, 2,
				0, 2, 3,
			};
			
			square_mesh = mesh_create(array_size(vertices), vertices, array_size(indices), indices);
		}
		
		{
			// Circle 
			
			// The circle is just a regular polygon with N sides.
			constexpr int N = 20; // Number of sides/triangles/ outer vertices.
			constexpr int vertex_count = 1 + N; // center + one vertex per side.
			constexpr int index_count = N * 3; // One triangle (center and two vertices) per side.
			
			Vertex vertices[vertex_count];
			uint32_t indices[index_count];
			
			// Center
			vertices[0] = {.position = {0, 0, 0}, .color = {1, 1, 1, 1}, .uv = {0.5, 0.5}};
			
			// Generate one vertex per side
			for (int i = 1; i < vertex_count; i += 1) {
				constexpr float dangle = (2.0f * PI) / (float) N;
				
				float angle = dangle * i;
				float x = 0.5f * cos(angle);
				float y = 0.5f * sin(angle);
				
				vertices[i] = {
					.position = {x, y, 0}, 
					.color = {1, 1, 1, 1}, 
					.uv = {x + 0.5f, y + 0.5f}
				};
			}
			
			// Generate the indices for each triangle 
			for (int t = 0; t <= N; t += 1) {
				indices[3 * t + 0] = 0;
				indices[3 * t + 1] = t + 1;
				indices[3 * t + 2] = t + 2;
			}
			
			indices[index_count - 1] = 1;
			
			circle_mesh = mesh_create(array_size(vertices), vertices, array_size(indices), indices);	
		}
	}
	
	ParticleParams params = {};
	
	params.physics.gravity = {0, -1};
	params.physics.friction = 0.999f;
	
	params.spawn.position = {
		.coordinate_system = CoordinateSystem::RECTANGULAR,
		.x = {Distribution::UNIFORM, -0.1, +0.1},
		.y = {Distribution::UNIFORM, -0.25, +0.25},
	};
	params.spawn.velocity = {
		.coordinate_system = CoordinateSystem::RECTANGULAR,
		.x = {Distribution::UNIFORM, -1, +1},
		.y = {Distribution::UNIFORM, 1, 1},
	};
	params.spawn.life = {Distribution::UNIFORM, 1, 3};
	params.spawn.color = {
		.color_system = ColorSystem::RGB,
		.red   = {Distribution::UNIFORM, 0.8, 1},
		.green = {Distribution::UNIFORM, 0.3, 1},
		.blue  = {Distribution::UNIFORM, 0.3, 1},
		.alpha = {Distribution::UNIFORM, 0.8, 1},
	};
	
	params.spawn.scale = {Distribution::UNIFORM, 0.001, 0.01};
	
	the_system = particle_system_create(1000);
	
	for (int i = 0; i < the_system->count; i += 1) {
		Particle* p = &the_system->particles[i];
		
		p->position.xy = random_get(params.spawn.position);
		p->scale = random_get(params.spawn.scale);
		p->color = random_get(params.spawn.color);
		
		p->velocity.xy = random_get(params.spawn.velocity) * 0.2;
	}
	
	return true;
}

mat4 orthographic(float left, float right, float top, float bottom, float near, float far) {
	float dx = right - left;
	float dy = top - bottom;
	float dz = far - near;
	
	float sx = 2.0f / dx;
	float sy = 2.0f / dy;
	float sz = 2.0f / dz;
	
	float tx = -(right + left) / dx;
	float ty = -(top + bottom) / dy;
	float tz = -(far + near)   / dz;
	
	return {
		sx, 0, 0, tx,
		0, sy, 0, ty,
		0, 0, sz, tz,
		0, 0, 0, 1,
	};
}

void do_frame() {
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	
	for (int i = 0; i < the_system->count; i += 1) {
		Particle* p = &the_system->particles[i];
		p->position += p->velocity * global_time.dt;
	}
	
	ShaderConstants constants;
	constants.projection = orthographic(-0.8f, +0.8f, +0.5f, -0.5f, -1, +1);
	particle_system_upload_and_render(the_system, circle_mesh, &constants);
}


