#pragma once

#include <math.h>
#include <xmmintrin.h>

#define PI 3.14159265358979323846

struct vec2f {
	float x;
	float y;
};

vec2f operator+(vec2f a, vec2f b);
vec2f operator-(vec2f a, vec2f b);
vec2f operator*(vec2f a, vec2f b);
vec2f operator/(vec2f a, vec2f b);

vec2f operator*(vec2f v, float f);
vec2f operator*(float f, vec2f v);

vec2f operator/(vec2f v, float f);

void operator+=(vec2f& a, vec2f b);
void operator-=(vec2f& a, vec2f b);
void operator*=(vec2f& a, vec2f b);
void operator/=(vec2f& a, vec2f b);

void operator*=(vec2f& v, float f);
void operator/=(vec2f& v, float f);

union vec3f {
	struct {
		float x;
		float y;
		float z;
	};
	vec2f xy;
};

vec3f operator+(vec3f a, vec3f b);
vec3f operator-(vec3f a, vec3f b);
vec3f operator*(vec3f a, vec3f b);
vec3f operator/(vec3f a, vec3f b);

vec3f operator*(vec3f v, float f);
vec3f operator*(float f, vec3f v);

vec3f operator/(vec3f v, float f);

void operator+=(vec3f& a, vec3f b);
void operator-=(vec3f& a, vec3f b);
void operator*=(vec3f& a, vec3f b);
void operator/=(vec3f& a, vec3f b);

void operator*=(vec3f& v, float f);
void operator/=(vec3f& v, float f);

union vec4f {
	struct {
		float x;
		float y;
		float z;
		float w;
	};
	vec3f xyz;
	vec2f xy;
};

vec4f operator+(vec4f a, vec4f b);
vec4f operator-(vec4f a, vec4f b);
vec4f operator*(vec4f a, vec4f b);
vec4f operator/(vec4f a, vec4f b);

vec4f operator*(vec4f v, float f);
vec4f operator*(float f, vec4f v);

vec4f operator/(vec4f v, float f);

void operator+=(vec4f& a, vec4f b);
void operator-=(vec4f& a, vec4f b);
void operator*=(vec4f& a, vec4f b);
void operator/=(vec4f& a, vec4f b);

void operator*=(vec4f& v, float f);
void operator/=(vec4f& v, float f);

union mat4f {
	float m[4][4];
};

vec4f operator*(mat4f m, vec4f v);
mat4f operator*(mat4f a, mat4f b);

mat4f orthographic(float left, float right, float top, float bottom, float near = -1, float far = +1);