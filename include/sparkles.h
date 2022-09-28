#pragma once

#include <inttypes.h>

#ifndef SPARKLES_ASSERT
#include <assert.h>
#define SPARKLES_ASSERT(condition) assert(condition)
#endif

#ifndef SPARKLES_LOG
#include <stdio.h>
#define SPARKLES_LOG(...) { printf(__VA_ARGS__); fflush(stdout); }
#endif

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
	
	//
	// Graphics stuff
	//
	
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
	
	enum class ShaderLanguage {
		GLSL,
	};
	
	enum class ShaderType {
		VERTEX,
		PIXEL,
	};
	
	enum class TextureFormat {
		NONE,
		
		RGBA_UINT8,
		RGBA_FLOAT16,
	};
	
	// These structs are defined by the graphics backend.
	struct Shader;
	struct Texture;
	struct Mesh; 
	struct RenderTarget;
	
	struct RenderState {
		Shader* vertex_shader = nullptr;
		Shader* pixel_shader = nullptr;
		RenderTarget* render_target = nullptr;
		
		Texture* texture0 = nullptr;
		
		// Shader constants
		mat4 projection = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1,
			
		};
	};
	
	//
	// Basic API
	// 
	bool initialize();
	ParticleSystem* particle_system_create(uint32_t particle_count);
	void particle_system_upload_and_render(ParticleSystem* system, Mesh* mesh, RenderState* render_state);
	
	//
	// Graphics Utility
	//
	
	// Shader
	Shader* shader_create(ShaderLanguage language, ShaderType type, const char* shader_source_code);
	// #todo: shader_hotload
	// #todo: shader_destroy
	
	// Texture 
	Texture* texture_create(TextureFormat format, uint32_t width, uint32_t height, void* image_data);
	// #todo: texture_destroy
	
	// Mesh
	Mesh* mesh_create(uint32_t vertex_count, uint32_t index_count, Vertex vertices[] = nullptr, uint32_t indices[] = nullptr);
	void mesh_upload(Mesh* mesh, uint32_t vertex_count, Vertex vertices[], uint32_t index_count, uint32_t indices[]);
	void mesh_render(Mesh* mesh, RenderState* render_state);
	// #todo: mesh_destroy
	
	// Render target
	RenderTarget* render_target_create(TextureFormat format, uint32_t width, uint32_t height);
	// #todo: mesh_destroy
}