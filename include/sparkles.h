#pragma once

#include <inttypes.h>

#ifndef SPARKLES_ASSERT
#include <assert.h>
#define SPARKLES_ASSERT(condition, ...) assert((condition));
#endif

#ifndef SPARKLES_LOG
#include <stdio.h>
#define SPARKLES_LOG(...) { printf(__VA_ARGS__); fflush(stdout); }
#endif

namespace Sparkles {
	
	// Below, we define our basic math data structures. 
	// Eventually we will provide macros to allow you, the user of this library, to use their own data structures.
	
	struct vec2 {
		float x;
		float y;
		
		constexpr vec2() : x(0), y(0) {}
		constexpr vec2(float x, float y) : x(x), y(y) {}
	};
	
	union vec3 {
		struct {
			float x;
			float y;
			float z;
		};
		vec2 xy;
		
		constexpr vec3() : x(0), y(0), z(0) {}
		constexpr vec3(float x, float y, float z) : x(x), y(y), z(z) {}
		constexpr vec3(vec2 xy, float z) : x(xy.x), y(xy.y), z(z) {}
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
		
		constexpr vec4() : x(0), y(0), z(0), w(0) {}
		constexpr vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
		constexpr vec4(vec2 xy, float z, float w) : x(xy.x), y(xy.y), z(z), w(w) {}
		constexpr vec4(vec3 xyz, float w) : x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}
	};	
	
	union mat4 {
		float m[4][4];
	};
	
	union Rect {
		struct {
			float x;
			float y;
			float w;
			float h;
		};
		struct {
			vec2 position;
			vec2 size;
		};
	};
	
	//
	// This is our general particle structure.
	// All the fields here are avaliable to be changed in the CPU and used in the GPU shaders.
	//
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
		
		constexpr Vertex() : x(0), y(0), z(0), r(0), g(0), b(0), a(0), u(0), v(0) {}
		constexpr Vertex(vec3 position, vec4 color, vec2 uv) : position(position), color(color), uv(uv) {}
		constexpr Vertex(float x, float y, float z, float r, float g, float b, float a, float u, float v) : x(x), y(y), z(z), r(r), g(g), b(b), a(a), u(u), v(v) {}
	};
	
	static_assert(sizeof(Vertex) == 9 * sizeof(float), "Wrong vertex size!");
	
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
		
		ALPHA_FLOAT32,
	};
	
	// These structs are defined by the graphics backend.
	
	struct Shader;
	
	struct Texture {
		int width;
		int height;
	};
	
	struct Mesh;
	
	struct RenderTarget {
		int width;
		int height;
	};
	
	struct RenderState {
		Shader* vertex_shader = nullptr;
		Shader* pixel_shader = nullptr;
		RenderTarget* render_target = nullptr;
		
		Texture* texture0 = nullptr;
		
		Rect viewport = {0, 0, 0, 0};
		
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
	bool            initialize();
	ParticleSystem* particle_system_create(uint32_t particle_count);
	void            particle_system_upload_and_render(ParticleSystem* system, Mesh* mesh, RenderState* render_state);
	
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
	void  mesh_upload(Mesh* mesh, uint32_t vertex_count, Vertex vertices[], uint32_t index_count, uint32_t indices[]);
	void  mesh_render(Mesh* mesh, RenderState* render_state, int32_t index_count = -1);
	// #todo: mesh_destroy
	
	// Render target
	RenderTarget* render_target_create(TextureFormat format, uint32_t width, uint32_t height);
	Texture*      render_target_flush(RenderTarget* render_target);
	void          render_target_clear(RenderTarget* render_target, vec4 color); // nullptr means clearing our window's backbuffer.
	
	// #todo: render_target_destroy
}