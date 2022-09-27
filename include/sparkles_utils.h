#pragma once

#include "sparkles.h"

namespace Sparkles {
	
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
	
	struct SpawnParams {
		RandomVec2 position;
		RandomScalar scale;
		RandomColor color;
		RandomVec2 velocity;
		RandomScalar life;
	};
	
	struct PhysicsParams {
		vec3 gravity;
		float friction;
	};
	
	// 
	// Simple simulation functions
	//
	void particle_simulate(Particle* particle, PhysicsParams* physics, float dt);
	void particle_spawn(Particle* particle, SpawnParams* spawn);
	
	//
	// Utility functions
	//
	float random_get_uniform();
	float random_get(RandomScalar spec);
	vec2 random_get(RandomVec2 spec);
	vec4 random_get(RandomColor spec);
	
	
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
	
}