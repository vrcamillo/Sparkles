#include <math.h>

// #random_number_cleanup
#include <stdlib.h> // srand
#include <time.h> // time

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
	
	Texture* texture_generate_light_mask(int width, int height, float brightness_factor, float brightness_cap) {
		float* data = new float[width * height]; // #memory_cleanup
		
		for (int j = 0; j < height; j += 1) {
			for (int i = 0; i < width; i += 1) {
				float x = (i / (width * 0.5))  - 1.0f;
				float y = (j / (height * 0.5)) - 1.0f;
				float r2 = x * x + y * y;
				float value = brightness_factor / r2;
				if (value > brightness_cap) value = brightness_cap;
				
				data[i + j * width] = value;
			}
		}
		
		Texture* result = texture_create(TextureFormat::ALPHA_FLOAT32, width, height, data);
		delete[] data;
		
		return result;
	}
	
	float random_get() {
		
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
	
	float random_get1(Range1 range) {
		float t = random_get();
		return (1 - t) * range.min + t * range.max;
	}
	
	vec2 random_get2(Range2 range) {
		float x = random_get1({range.min.x, range.max.x});
		float y = random_get1({range.min.y, range.max.y});
		
		switch (range.coords) {
		  case Coords2D::CARTESIAN: return {x, y};	
		  case Coords2D::POLAR: {
				float radius = x;
				float angle = y;
				return {radius * (float) cos(angle), radius * (float) sin(angle)};
			}
			
		  default: assert(false);
		}
		
		return {x, y};
	}
	
	vec3 random_get3(Range3 range) {
		float x = random_get1({range.min.x, range.max.x});
		float y = random_get1({range.min.y, range.max.y});
		float z = random_get1({range.min.z, range.max.z});
		
		switch (range.coords) {
		  case Coords3D::CARTESIAN: return {x, y, z};	
		  case Coords3D::SPHERICAL: {
				float radius  = x;
				float polar   = y;
				float azimuth = z;
				
				float sin_azimuth = sin(azimuth);
				float cos_azimuth = cos(azimuth);
				
				return {
					(float) (radius * sin_azimuth * cos(polar)), 
					(float) (radius * sin_azimuth * sin(polar)), 
					(float) (radius * cos_azimuth)
				};
			}
			
		  case Coords3D::CYLINDRICAL: {
				float radius = x;
				float azimuth = y;
				
				return {
					(float) (radius * cos(azimuth)),
					(float) (radius * sin(azimuth)),
					z
				};
			}
			
		  default: assert(false);
		}
		
		return {x, y, z};
	}
	
	vec4 random_get4(Range4 range) {
		float x = random_get1({range.min.x, range.max.x});
		float y = random_get1({range.min.y, range.max.y});
		float z = random_get1({range.min.z, range.max.z});
		float w = random_get1({range.min.w, range.max.w});
		
		return {x, y, z, w};
	}
	
	
	/*
	void particle_simulate(Particle* p, ParticlePhysicsParams* physics, float dt) {
		p->velocity += physics->gravity * dt;
		p->position += p->velocity * dt;
		p->velocity *= physics->friction; // #temporary: This should not be frame rate dependant!
		p->life -= dt;
	}
	*/
	void particle_spawn(Particle* p, ParticleSpawnParams* spawn) {
		p->position.xy = random_get2(spawn->position);
		p->position.z = 0;
		p->scale = random_get1(spawn->scale);
		p->color = random_get4(spawn->color);
		p->velocity.xy = random_get2(spawn->velocity) * 0.2;
		p->life = random_get1(spawn->life);
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