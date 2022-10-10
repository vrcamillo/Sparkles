#include <math.h>

// #random_number_cleanup
#include <stdlib.h> // srand
#include <time.h> // time

#include "sparkles.h"
#include "sparkles_utils.h"

namespace Sparkles {
	MeshBuilder mesh_builder_create(uint32_t vertex_capacity, uint32_t index_capacity) {
		MeshBuilder result = {};
		result.vertex_capacity = vertex_capacity;
		result.index_capacity  = index_capacity;
		result.vertices = new Vertex[vertex_capacity];
		result.indices  = new uint32_t[index_capacity];
		return result;
	}
	
	void mesh_builder_clear(MeshBuilder* builder) {
		builder->vertex_cursor = 0;
		builder->index_cursor  = 0;
	}
		
	Vertex* put_vertices(MeshBuilder* builder, int count) {
		SPARKLES_ASSERT(builder->vertex_cursor + count <= builder->vertex_capacity, "Too many vertices.");
		auto result = &builder->vertices[builder->vertex_cursor];
		builder->vertex_cursor += count;
		return result;
	}
	
	void put_indices(MeshBuilder* builder, Vertex* first_vertex, int index0, int index1, int index2) {
		SPARKLES_ASSERT(builder->index_cursor + 3 <= builder->index_capacity, "Too many indices.");
		int offset = (first_vertex - builder->vertices);
		builder->indices[builder->index_cursor + 0] = index0 + offset;
		builder->indices[builder->index_cursor + 1] = index1 + offset;
		builder->indices[builder->index_cursor + 2] = index2 + offset;
		builder->index_cursor += 3;
	}
	
	void put_rect(MeshBuilder* builder, Rect rect) {		
		float x0 = rect.x;
		float x1 = rect.x + rect.w;
		float y0 = rect.y;
		float y1 = rect.y + rect.h;
		
		auto vertices = put_vertices(builder, 4);
		vertices[0] = {x0, y0, 0, 1, 1, 1, 1, 0, 0};
		vertices[1] = {x1, y0, 0, 1, 1, 1, 1, 1, 0};
		vertices[2] = {x1, y1, 0, 1, 1, 1, 1, 1, 1};
		vertices[3] = {x0, y1, 0, 1, 1, 1, 1, 0, 1};
		
		put_indices(builder, &vertices[0], 0, 1, 2);
		put_indices(builder, &vertices[0], 0, 2, 3);
	}
	
	void put_regular_polygon(MeshBuilder* builder, vec2 center, float radius, int number_of_sides) {
		int vertex_count = 1 + number_of_sides; // center + one vertex per side.
		int index_count = number_of_sides * 3; // One triangle (center and two vertices) per side.
				
		auto vertices = put_vertices(builder, vertex_count);
		
		// Center
		vertices[0] = {
			center.x, center.y, 0, 
			1, 1, 1, 1, 
			0.5, 0.5
		};
		
		float dangle = (2.0f * PI) / (float) number_of_sides;
		
		// Generate one vertex per side
		for (int i = 1; i < vertex_count; i += 1) {
			
			float angle = dangle * i;
			float x = cos(angle);
			float y = sin(angle);
			
			vertices[i] = {
				center.x + x * radius, center.y + y * radius, 0, 
				1, 1, 1, 1, 
				0.5f * x + 0.5f, 0.5f * y + 0.5f
			};
		}
		
		// Generate the indices for each triangle 
		for (int t = 0; t < number_of_sides; t += 1) {
			put_indices(builder, &vertices[0], 0, t + 1, t + 2);
		}
		
		put_indices(builder, &vertices[0], 0, number_of_sides, 1);
	}
	
	void put_bezier(MeshBuilder* builder, CubicBezier* curve, float line_width, int number_of_points) {
		float dt = 1.0f / number_of_points;
		for (int i = 0; i < number_of_points; i += 1) {			
			float t = dt * i;
			vec2 center = bezier_get_position(*curve, t);
			vec2 tangent = normalize(bezier_get_tangent(*curve, t));
			vec2 offset = (line_width * 0.5f) * rotate2(tangent, TAU * 0.25);
			
			vec2 p0 = center + offset;
			vec2 p1 = center - offset;
			
			auto vertices = put_vertices(builder, 2);
			vertices[0] = {p0.x, p0.y, 0, 1, 1, 1, 1, 0, 0};		
			vertices[1] = {p1.x, p1.y, 0, 1, 1, 1, 1, 0, 0};
			
			if (i < number_of_points - 1) {
				put_indices(builder, vertices, 0, 1, 2);
				put_indices(builder, vertices, 1, 2, 3);
			}
		}	
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
	
	void particle_spawn(Particle* p, ParticleSpawnParams* spawn) {
		p->position.xy = random_get2(spawn->position);
		p->position.z = 0;
		p->scale = random_get1(spawn->scale);
		p->color = random_get4(spawn->color);
		p->velocity.xy = random_get2(spawn->velocity) * 0.2;
		p->life = random_get1(spawn->life);
	}
	
	float norm2(vec2 v) {
		return v.x * v.x + v.y * v.y;
	}
	
	float norm(vec2 v) {
		return sqrt(v.x * v.x + v.y * v.y);		
	}

	vec2 normalize(vec2 v, float epsilon) {
		float length = norm(v);
		if (length > epsilon) {
			v.x /= length;
			v.y /= length;
		}
		return v;
	}
	
	vec2 lerp(vec2 a, vec2 b, float t) {
		return (1 - t) * a + t * b;
	}
	
	vec3 lerp(vec3 a, vec3 b, float t) {
		return (1 - t) * a + t * b;
	}
	
	vec4 lerp(vec4 a, vec4 b, float t) {
		return (1 - t) * a + t * b;
	}
	
	vec2 rotate2(vec2 v, float radians) {
		float s = sin(radians);
		float c = cos(radians);
		return {c * v.x - s * v.y, s * v.x + c * v.y};
	}
	
	vec2 bezier_get_position(CubicBezier curve, float t) {
		
		float t2 = t * t;
		float t3 = t2 * t;
		
		float s = 1 - t;
		float s2 = s * s;
		float s3 = s2 * s;
		
		vec2 p0 = curve.a;
		vec2 p1 = curve.a + curve.c0;
		vec2 p2 = curve.b + curve.c1;
		vec2 p3 = curve.b;
		
		return (s3) * p0 + (3 * s2 * t) * p1 + (3 * s * t2) * p2 + t3 * p3;
	}
	
	vec2 bezier_get_tangent(CubicBezier curve, float t) {
		float t2 = t * t;
		
		float s = 1 - t;
		float s2 = s * s;
		
		vec2 p0 = curve.a;
		vec2 p1 = curve.a + curve.c0;
		vec2 p2 = curve.b + curve.c1;
		vec2 p3 = curve.b;
		
		return (3 * s2) * (p1 - p0) + (6 * s * t) * (p2 - p1) + (3 * t2) * (p3 - p2);
	}
	
	float bezier_get_length(CubicBezier curve, float start_t, float end_t, float dt) {
		return 0;
	}
	
	mat4 mat4_identity() {
		return {
			1, 0, 0, 0,
			0, 1, 0, 0, 
			0, 0, 1, 0,
			0, 0, 0, 1,
		};
	}
	
	
	mat4 mat4_translation(vec3 offset) {
		return {
			1, 0, 0, offset.x,
			0, 1, 0, offset.y, 
			0, 0, 1, offset.y,
			0, 0, 0, 1,
		};
	}
	
	mat4 mat4_scale(float scale) {
		return {
			scale, 0, 0, 0,
			0, scale, 0, 0, 
			0, 0, scale, 0,
			0, 0, 0, 1,
		};
	}
	
	vec2 polar(float radius, float angle) {
		return {radius * (float) cos(angle), radius * (float) sin(angle)};
	}
	
}