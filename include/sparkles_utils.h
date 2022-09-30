#pragma once

#include "sparkles.h"

namespace Sparkles {
	
	static constexpr float PI  = 3.1415926f;
	static constexpr float TAU = 6.2831853f;
	
	enum class Coords2D {
		CARTESIAN,
		POLAR,
	};
	
	enum class Coords3D {
		CARTESIAN,
		CYLINDRICAL,
		SPHERICAL,
	};
	
	enum class ColorSystem {
		RGB,
		HSB,
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
	
	constexpr Range2 polar(float min_radius, float max_radius, float min_angle, float max_angle) {
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
		vec2 p0;
		vec2 p1;
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
	
	// 
	// Simple simulation functions
	//
	void particle_spawn(Particle* particle, ParticleSpawnParams* spawn);
	
	//
	// Texture generation function
	//
	
	Texture* texture_generate_light_mask(int width, int height, float brightness_factor, float brightness_cap);
	
	//
	// Mesh generation functions
	//
	Mesh* mesh_generate_regular_polygon(int number_of_sides);
	Mesh* mesh_generate_quad(vec2 p0, vec2 p1);
	Mesh* mesh_generate_bezier(CubicBezier curve, int number_of_points, float line_width);
	
	//
	// Random number generator functions
	//
	float random_get();
	float random_get1(Range1 range);
	vec2 random_get2(Range2 range);
	vec3 random_get3(Range3 range);
	vec4 random_get4(Range4 range);
	
	// Math functions
	
#if !SPARKLES_USER_MATH_TYPES 
	// If the user didn't define their vector types, we provide operator overloads for our vectors for convenience.
	
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
	
#endif
	
	float norm2(vec2 v);
	
	float norm(vec2 v);
	
	vec2 normalize(vec2 v);
	
	vec2 lerp(vec2 a, vec2 b, float t);
	vec3 lerp(vec3 a, vec3 b, float t);
	vec4 lerp(vec4 a, vec4 b, float t);
	
	vec2 rotate2(vec2 v, float radians);
	
	vec2 bezier_get_position(CubicBezier curve, float t);
	vec2 bezier_get_tangent(CubicBezier curve, float t);
	float bezier_get_length(CubicBezier curve, float start_t = 0, float end_t = 1, float dt = 0.01);
	
	mat4 mat4_identity();
	mat4 orthographic(float left, float right, float top, float bottom, float near, float far);
	
}