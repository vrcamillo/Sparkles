#pragma once

#include "sparkles.h"

namespace Sparkles {
	
	static constexpr float PI  = 3.1415926f;
	static constexpr float TAU = 6.2831853f;
	
	enum class Distribution {
		UNIFORM, // Every value in the sample has the same chance of being chosen.
	};
	
	enum class CoordinateSystem {
		RECTANGULAR,
		POLAR,
	};
	
	enum class ColorSystem {
		RGB,
		HSB,
	};
	
	struct RandomScalar {
		Distribution distribution;
		float min;
		float max;
	};
	
	struct RandomVec2 {
		CoordinateSystem coordinate_system;
		
		union {
			struct {
				RandomScalar x;
				RandomScalar y;
			};
			struct {
				RandomScalar angle;
				RandomScalar radius;
			};
		};
		
		vec2 offset;
	};
	
	struct RandomColor {
		ColorSystem color_system;
		
		union {
			struct {
				RandomScalar red;
				RandomScalar green;
				RandomScalar blue;
			};
			struct {
				RandomScalar hue;
				RandomScalar saturation;
				RandomScalar brightness;
			};
		};
		
		RandomScalar alpha;
	};
	
	struct ParticleSpawnParams {
		RandomVec2 position;
		RandomScalar scale;
		RandomColor color;
		RandomVec2 velocity;
		RandomScalar life;
	};
	
	struct ParticlePhysicsParams {
		vec3 gravity;
		float friction;
	};
	
	constexpr RandomScalar uniform1(float min, float max) {
		return {Distribution::UNIFORM, min, max};
	}
	
	constexpr RandomVec2 uniform2_rect(vec2 min, vec2 max) {
		return {
			.coordinate_system = CoordinateSystem::RECTANGULAR,
			.x = {Distribution::UNIFORM, min.x, max.x},
			.y = {Distribution::UNIFORM, min.y, max.y},
		};
	}
	
	constexpr RandomVec2 uniform2_polar(float angle_min, float angle_max, float radius_min, float radius_max, vec2 offset = {}) {
		return {
			.coordinate_system = CoordinateSystem::POLAR,
			.angle  = {Distribution::UNIFORM, angle_min, angle_max},
			.radius = {Distribution::UNIFORM, radius_min, radius_max},
			.offset = offset,
		};
	}
	
	constexpr RandomColor uniform_rgba(vec4 color_min, vec4 color_max) {
		return {
			.color_system = ColorSystem::RGB,
			.red   = {Distribution::UNIFORM, color_min.x, color_max.x},
			.green = {Distribution::UNIFORM, color_min.y, color_max.y},
			.blue  = {Distribution::UNIFORM, color_min.z, color_max.z},
			.alpha = {Distribution::UNIFORM, color_min.w, color_max.w},
		};
	}
	
	// 
	// Simple simulation functions
	//
	void particle_simulate(Particle* particle, ParticlePhysicsParams* physics, float dt);
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
	
	//
	// Random number generator functions
	//
	float random_get_uniform();
	float random_get(RandomScalar spec);
	vec2 random_get(RandomVec2 spec);
	vec4 random_get(RandomColor spec);
	
	// Math functions
	mat4 mat4_identity();
	
#if !SPARKLES_USER_MATH_TYPES // If the user didn't define their vector types, we provide operator overloads for our vectors for convenience.
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

	mat4 orthographic(float left, float right, float top, float bottom, float near, float far);
	
}