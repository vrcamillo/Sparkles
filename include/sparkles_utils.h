#pragma once

#include "sparkles.h"

namespace Sparkles {
	
	static constexpr float PI  = 3.1415926f;
	static constexpr float TAU = 6.2831853f;
	
	// In this library we use the order convention often used in physics.
	// We may change this in the future.
	
	enum class Coords2D {
		CARTESIAN, // (x, y)
		POLAR,     // (radius, azimuth)
	};
	
	enum class Coords3D {
		CARTESIAN,   // (x, y, z)
		CYLINDRICAL, // (radius, azimuth, height)
		SPHERICAL,   // (radius, polar, azimuth) 
	};
	
	enum class ColorSystem {
		RGB, // (red, green, blue, alpha)
		HSB, // (hue, saturation, brightness, alpha)
	};
	
	struct Range1 {
		float min = 0;
		float max = 1;
	};
	
	struct Range2 {
		vec2 min = {0, 0};
		vec2 max = {1, 1};
		Coords2D coords = Coords2D::CARTESIAN;
	};
	
	constexpr Range2 range_polar(float min_radius, float max_radius, float min_angle, float max_angle) {
		return {{min_radius, min_angle}, {max_radius, max_angle}, Coords2D::POLAR};
	}
	
	struct Range3 {
		vec3 min = {0, 0, 0};
		vec3 max = {1, 1, 1};
		Coords3D coords = Coords3D::CARTESIAN;
	};
	
	struct Range4 {
		vec4 min = {0, 0, 0, 0};
		vec4 max = {1, 1, 1, 1};
	};
	
	struct CubicBezier {
		vec2 a;
		vec2 b;
		vec2 c0;
		vec2 c1;
	};
	
	struct ParticleSpawnParams {
		Range2 position;
		Range1 scale;
		Range4 color;
		Range2 velocity;
		Range1 life;
	};
	
	struct MeshBuilder {
		uint32_t vertex_capacity;
		uint32_t vertex_cursor;
		Vertex* vertices;
		
		uint32_t index_capacity;
		uint32_t index_cursor;
		uint32_t* indices;
	};
	
	// 
	// Simple simulation functions
	//
	void particle_spawn(Particle* particle, ParticleSpawnParams* spawn);
	
	//
	// Texture generation function
	//
	
	Texture* texture_generate_light_mask(int width, int height, float brightness_factor, float brightness_cap);
	
	
	//
	// Geometry generation functions
	//
	
	MeshBuilder mesh_builder_create(uint32_t vertex_capacity, uint32_t index_capacity);
	// #todo mesh_builder_destroy()
	void mesh_builder_clear(MeshBuilder* builder);
	
	Vertex* put_vertices(MeshBuilder* builder, int count);
	void put_indices(MeshBuilder* builder, Vertex* first_vertex, int index0, int index1, int index2);
	
	void put_rect(MeshBuilder* builder, Rect rect);
	void put_regular_polygon(MeshBuilder* builder, vec2 center, float radius, int number_of_sides);
	void put_bezier(MeshBuilder* builder, CubicBezier* curve, float line_width, int number_of_points);
		
	//
	// Random number generator functions
	//
	float random_get();
	float random_get1(Range1 range);
	vec2 random_get2(Range2 range);
	vec3 random_get3(Range3 range);
	vec4 random_get4(Range4 range);
	
	// Math operator overloads
	
	// vec2
	vec2 operator+(vec2 a, vec2 b);
	vec2 operator-(vec2 a, vec2 b);
	vec2 operator*(vec2 a, vec2 b);
	vec2 operator/(vec2 a, vec2 b);
	
	vec2 operator*(vec2 v, float f);
	vec2 operator*(float f, vec2 v);
	
	vec2 operator/(vec2 v, float f);
	
	void operator+=(vec2& a, vec2 b);
	void operator-=(vec2& a, vec2 b);
	void operator*=(vec2& a, vec2 b);
	void operator/=(vec2& a, vec2 b);
	
	void operator*=(vec2& v, float f);
	void operator/=(vec2& v, float f);
	
	// vec3
	vec3 operator+(vec3 a, vec3 b);
	vec3 operator-(vec3 a, vec3 b);
	vec3 operator*(vec3 a, vec3 b);
	vec3 operator/(vec3 a, vec3 b);
	
	vec3 operator*(vec3 v, float f);
	vec3 operator*(float f, vec3 v);
	
	vec3 operator/(vec3 v, float f);
	
	void operator+=(vec3& a, vec3 b);
	void operator-=(vec3& a, vec3 b);
	void operator*=(vec3& a, vec3 b);
	void operator/=(vec3& a, vec3 b);
	
	void operator*=(vec3& v, float f);
	void operator/=(vec3& v, float f);
	
	// vec4
	vec4 operator+(vec4 a, vec4 b);
	vec4 operator-(vec4 a, vec4 b);
	vec4 operator*(vec4 a, vec4 b);
	vec4 operator/(vec4 a, vec4 b);
	
	vec4 operator*(vec4 v, float f);
	vec4 operator*(float f, vec4 v);
	
	vec4 operator/(vec4 v, float f);
	
	void operator+=(vec4& a, vec4 b);
	void operator-=(vec4& a, vec4 b);
	void operator*=(vec4& a, vec4 b);
	void operator/=(vec4& a, vec4 b);
	
	void operator*=(vec4& v, float f);
	void operator/=(vec4& v, float f);
	
	// mat4
	vec4 operator*(mat4 m, vec4 v);
	mat4 operator*(mat4 a, mat4 b);
	
	float norm2(vec2 v);
	float norm(vec2 v);
	
	vec2 normalize(vec2 v, float epsilon = 0.001);
	
	vec2 lerp(vec2 a, vec2 b, float t);
	vec3 lerp(vec3 a, vec3 b, float t);
	vec4 lerp(vec4 a, vec4 b, float t);
	
	vec2 polar(float radius, float angle);
	vec2 rotate2(vec2 v, float radians);
	
	vec2  bezier_get_position(CubicBezier curve, float t);
	vec2  bezier_get_tangent(CubicBezier curve, float t);
	float bezier_get_length(CubicBezier curve, float start_t = 0, float end_t = 1, float dt = 0.01);
	
	mat4 mat4_identity();
	mat4 mat4_translation(vec3 offset);
	mat4 mat4_scale(float scale);
	
	mat4 orthographic(float left, float right, float top, float bottom, float near, float far);
}