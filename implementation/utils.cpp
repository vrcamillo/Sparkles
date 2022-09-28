#include <math.h>

// #random_number_cleanup
#include <stdlib.h> // srand
#include <time.h> // time

#define _USE_MATH_DEFINES
#include <cmath>
// #include <math.h>
#define PI M_PI
#define TAU (2 * M_PI)

// #include "sparkles.h"
#include "sparkles_utils.h"

namespace Sparkles {
	
	
	Mesh* mesh_generate_regular_polygon(int number_of_sides) {
		constexpr int MAX_VERTICES = 128;
		constexpr int MAX_INDICES = 256;
		Vertex vertices[MAX_VERTICES];
		uint32_t indices[MAX_INDICES];
		
		int N = number_of_sides; // Number of sides/triangles/ outer vertices.
		int vertex_count = 1 + N; // center + one vertex per side.
		int index_count = N * 3; // One triangle (center and two vertices) per side.
		
		SPARKLES_ASSERT(vertex_count < MAX_VERTICES);
		SPARKLES_ASSERT(index_count < MAX_INDICES);
		
		// Center
		vertices[0] = {.position = {0, 0, 0}, .color = {1, 1, 1, 1}, .uv = {0.5, 0.5}};
		
		float dangle = (2.0f * PI) / (float) N;
		
		// Generate one vertex per side
		for (int i = 1; i < vertex_count; i += 1) {
			
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
		
		Mesh* result = mesh_create(vertex_count, index_count, vertices, indices);	
		return result;
	}
	
	Mesh* mesh_generate_quad(vec2 p0, vec2 p1) {
		Vertex vertices[4] = {
			{.position = {p0.x, p0.y, 0}, .color = {1, 1, 1, 1}, .uv = {0, 0}},
			{.position = {p1.x, p0.y, 0}, .color = {1, 1, 1, 1}, .uv = {1, 0}},
			{.position = {p1.x, p1.y, 0}, .color = {1, 1, 1, 1}, .uv = {1, 1}},
			{.position = {p0.x, p1.y, 0}, .color = {1, 1, 1, 1}, .uv = {0, 1}},
		};
		
		uint32_t indices[6] = {
			0, 1, 2,
			0, 2, 3,
		};
		
		Mesh* result = mesh_create(4, 6, vertices, indices);	
		return result;
	}
	
	float random_get_uniform() {
		
		static bool first = true;
		if (first) {
			// Initialize random number generator seed to the current time, so that every time we run the program we get different results.
			// Eventually we will want to be very careful about random numbers, but for the mean time, we just use c standard lib.
			// #random_number_cleanup
			srand(time(NULL));
			first = false;
		}
		
		float result = rand() / (float) RAND_MAX;
		return result;
	}
	
	float random_get(RandomScalar spec) {
		switch (spec.distribution) {
		  case Distribution::UNIFORM: return spec.min + random_get_uniform() * (spec.max - spec.min);
		  default: assert(false);
		}
		
		return 0;
	}
	
	vec2 random_get(RandomVec2 spec) {
		switch (spec.coordinate_system) {
		  case CoordinateSystem::RECTANGULAR: {
				float x = random_get(spec.x);
				float y = random_get(spec.y);
				return {x, y};
			} break;
			
		  case CoordinateSystem::POLAR: {
				float angle = random_get(spec.angle);
				float radius = random_get(spec.radius);
				return {radius * (float) cos(angle), radius * (float) sin(angle)};
			} break;
			
		  default: assert(false);
		}
		
		return {};
	}
	
	vec4 random_get(RandomColor spec) {
		switch (spec.color_system) {
		  case ColorSystem::RGB: {
				float red   = random_get(spec.red);
				float green = random_get(spec.green);
				float blue  = random_get(spec.blue);
				float alpha = random_get(spec.alpha);
				
				return {red, green, blue, alpha};
			} break;
			
		  case ColorSystem::HSB:
			assert(false); // #unimplemented
			return {};
			
		  default: assert(false);
		}
		
		return {};
	}
	
	
	void particle_simulate(Particle* p, PhysicsParams* physics, float dt) {
		p->velocity += physics->gravity * dt;
		p->position += p->velocity * dt;
		p->velocity *= physics->friction; // #temporary: This should not be frame rate dependant!
		p->life -= dt;
	}
	
	void particle_spawn(Particle* p, SpawnParams* spawn) {
		p->position.xy = random_get(spawn->position);
		p->position.z = 0;
		p->scale = random_get(spawn->scale);
		p->color = random_get(spawn->color);
		p->velocity.xy = random_get(spawn->velocity) * 0.2;
		p->life = random_get(spawn->life);
	}

	mat4 mat4_identity() {
		return {
			1, 0, 0, 0,
			0, 1, 0, 0, 
			0, 0, 1, 0,
			0, 0, 0, 1,
		};
	}
	
	
}