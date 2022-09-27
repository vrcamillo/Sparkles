#pragma once

#include <inttypes.h>

// #define assert(condition) 

namespace Sparkles {
	
#if SPARKLES_USER_MATH_TYPES
	using vec2 = SPARKLES_USER_VEC2;
	using vec3 = SPARKLES_USER_VEC3;
	using vec4 = SPARKLES_USER_VEC4;
	using mat4 = SPARKLES_USER_MAT4;
#else
	struct vec2 {
		float x;
		float y;
	};
	
	union vec3 {
		struct {
			float x;
			float y;
			float z;
		};
		vec2 xy;
	};
	
	union vec4 {
		struct {
			float x;
			float y;
			float z;
			float w;
		};
		vec3 xyz;
		vec2 xy;
	};	
	
	union mat4 {
		float m[4][4];
	};
	
#endif
	
	struct Particle {
		vec3 position;
		float scale;
		vec4 color;
		vec3 velocity;
		float life;
	};
	
	struct ParticleSystem {
		uint32_t count;
		Particle* particles;
	};
	
	// In the future, we may support other vertex formats, but for now, this is the default one.
	union Vertex {
		struct {
			vec3 position;
			vec4 color;
			vec2 uv;
		};
		struct {
			float x;
			float y;
			float z;
			
			float r;
			float g;
			float b;
			float a;
			
			float u;
			float v;
		};
	};
	
	static_assert(sizeof(Vertex) == 9 * sizeof(float));
	
	// These structs are defined by the graphics backend.
	struct Mesh; 
	struct ShaderPipeline;
	
	struct ShaderConstants {
		mat4 projection;
	};
	
	//
	// Basic API
	// 
	
	// Particle system.
	ParticleSystem* particle_system_create(uint32_t particle_count);
	void particle_system_upload_and_render(ParticleSystem* system, Mesh* mesh, ShaderConstants* constants, ShaderPipeline* shader_pipeline = nullptr);
	
	// Mesh initialization
	Mesh* mesh_create(uint32_t vertex_count, Vertex vertices[], uint32_t index_count, uint32_t indices[]);
	void mesh_destroy(Mesh* mesh);
	
	// Shader initialization
	ShaderPipeline* shader_pipeline_create_glsl(const char* glsl_vertex_shader_source, const char* glsl_pixel_shader_source); // Only supporting GLSL for now.
	ShaderPipeline* shader_pipeline_destroy(ShaderPipeline* shader_pipeline);
}