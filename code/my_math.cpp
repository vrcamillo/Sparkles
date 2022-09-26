#include "my_math.h"

// vec2f

vec2f operator+(vec2f a, vec2f b) {
	vec2f result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	return result;
}

vec2f operator-(vec2f a, vec2f b) {
	vec2f result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	return result;	
}

vec2f operator*(vec2f a, vec2f b) {
	vec2f result;
	result.x = a.x * b.x;
	result.y = a.y * b.y;
	return result;
}

vec2f operator/(vec2f a, vec2f b) {
	vec2f result;
	result.x = a.x / b.x;
	result.y = a.y / b.y;
	return result;
}

vec2f operator*(vec2f v, float f) {
	vec2f result;
	result.x = v.x * f;
	result.y = v.y * f;
	return result;
}

vec2f operator*(float f, vec2f v) {
	vec2f result;
	result.x = v.x * f;
	result.y = v.y * f;
	return result;	
}

vec2f operator/(vec2f v, float f) {
	vec2f result;
	result.x = v.x / f;
	result.y = v.y / f;
	return result;
}

void operator+=(vec2f& a, vec2f b) {
	a.x += b.x;
	a.y += b.y;
}

void operator-=(vec2f& a, vec2f b) {
	a.x -= b.x;
	a.y -= b.y;
}

void operator*=(vec2f& a, vec2f b) {
	a.x *= b.x;
	a.y *= b.y;
}

void operator/=(vec2f& a, vec2f b) {
	a.x /= b.x;
	a.y /= b.y;
}

void operator*=(vec2f& v, float f) {
	v.x *= f;
	v.y *= f;
}

void operator/=(vec2f& v, float f) {
	v.x /= f;
	v.y /= f;
}

// vec3f

vec3f operator+(vec3f a, vec3f b) {
	vec3f result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	result.z = a.z + b.z;
	return result;
}

vec3f operator-(vec3f a, vec3f b) {
	vec3f result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	result.z = a.z - b.z;
	return result;	
}

vec3f operator*(vec3f a, vec3f b) {
	vec3f result;
	result.x = a.x * b.x;
	result.y = a.y * b.y;
	result.z = a.z * b.z;
	return result;
}

vec3f operator/(vec3f a, vec3f b) {
	vec3f result;
	result.x = a.x / b.x;
	result.y = a.y / b.y;
	result.z = a.z / b.z;
	return result;
}

vec3f operator*(vec3f v, float f) {
	vec3f result;
	result.x = v.x * f;
	result.y = v.y * f;
	result.z = v.z * f;
	return result;
}

vec3f operator*(float f, vec3f v) {
	vec3f result;
	result.x = v.x * f;
	result.y = v.y * f;
	result.z = v.z * f;
	return result;	
}

vec3f operator/(vec3f v, float f) {
	vec3f result;
	result.x = v.x / f;
	result.y = v.y / f;
	result.z = v.z / f;
	return result;
}

void operator+=(vec3f& a, vec3f b) {
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
}

void operator-=(vec3f& a, vec3f b) {
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
}

void operator*=(vec3f& a, vec3f b) {
	a.x *= b.x;
	a.y *= b.y;
	a.z *= b.z;
}

void operator/=(vec3f& a, vec3f b) {
	a.x /= b.x;
	a.y /= b.y;
	a.z /= b.z;
}

void operator*=(vec3f& v, float f) {
	v.x *= f;
	v.y *= f;
	v.z *= f;
}

void operator/=(vec3f& v, float f) {
	v.x /= f;
	v.y /= f;
	v.z /= f;
}

// vec4f

vec4f operator+(vec4f a, vec4f b) {
	vec4f result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	result.z = a.z + b.z;
	result.w = a.w + b.w;
	return result;
}

vec4f operator-(vec4f a, vec4f b) {
	vec4f result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	result.z = a.z - b.z;
	result.w = a.w - b.w;
	return result;	
}

vec4f operator*(vec4f a, vec4f b) {
	vec4f result;
	result.x = a.x * b.x;
	result.y = a.y * b.y;
	result.z = a.z * b.z;
	result.w = a.w * b.w;
	return result;
}

vec4f operator/(vec4f a, vec4f b) {
	vec4f result;
	result.x = a.x / b.x;
	result.y = a.y / b.y;
	result.z = a.z / b.z;
	result.w = a.w / b.w;
	return result;
}

vec4f operator*(vec4f v, float f) {
	vec4f result;
	result.x = v.x * f;
	result.y = v.y * f;
	result.z = v.z * f;
	result.w = v.w * f;
	return result;
}

vec4f operator*(float f, vec4f v) {
	vec4f result;
	result.x = v.x * f;
	result.y = v.y * f;
	result.z = v.z * f;
	result.w = v.w * f;
	return result;	
}

vec4f operator/(vec4f v, float f) {
	vec4f result;
	result.x = v.x / f;
	result.y = v.y / f;
	result.z = v.z / f;
	result.w = v.w / f;
	return result;
}

void operator+=(vec4f& a, vec4f b) {
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	a.w += b.w;
}

void operator-=(vec4f& a, vec4f b) {
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	a.w -= b.w;
}

void operator*=(vec4f& a, vec4f b) {
	a.x *= b.x;
	a.y *= b.y;
	a.z *= b.z;
	a.w *= b.w;
}

void operator/=(vec4f& a, vec4f b) {
	a.x /= b.x;
	a.y /= b.y;
	a.z /= b.z;
	a.w /= b.w;
}

void operator*=(vec4f& v, float f) {
	v.x *= f;
	v.y *= f;
	v.z *= f;
	v.w *= f;
}

void operator/=(vec4f& v, float f) {
	v.x /= f;
	v.y /= f;
	v.z /= f;
	v.w /= f;
}

// mat4f

vec4f operator*(mat4f m, vec4f v) {
	vec4f result;
	result.x = m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z + m.m[0][3] * v.w;
	result.y = m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z + m.m[1][3] * v.w;
	result.z = m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z + m.m[2][3] * v.w;
	result.w = m.m[3][0] * v.x + m.m[3][1] * v.y + m.m[3][2] * v.z + m.m[3][3] * v.w;
	return result;
}

mat4f operator*(mat4f a, mat4f b) {
	mat4f result;
	
	result.m[0][0] = a.m[0][0] * b.m[0][0] + a.m[0][1] * b.m[1][0] + a.m[0][2] * b.m[2][0] + a.m[0][3] * b.m[3][0];
	result.m[0][1] = a.m[0][0] * b.m[0][1] + a.m[0][1] * b.m[1][1] + a.m[0][2] * b.m[2][1] + a.m[0][3] * b.m[3][1];
	result.m[0][2] = a.m[0][0] * b.m[0][2] + a.m[0][1] * b.m[1][2] + a.m[0][2] * b.m[2][2] + a.m[0][3] * b.m[3][2];
	result.m[0][3] = a.m[0][0] * b.m[0][3] + a.m[0][1] * b.m[1][3] + a.m[0][2] * b.m[2][3] + a.m[0][3] * b.m[3][3];
	
	result.m[1][0] = a.m[1][0] * b.m[0][0] + a.m[1][1] * b.m[1][0] + a.m[1][2] * b.m[2][0] + a.m[1][3] * b.m[3][0];
	result.m[1][1] = a.m[1][0] * b.m[0][1] + a.m[1][1] * b.m[1][1] + a.m[1][2] * b.m[2][1] + a.m[1][3] * b.m[3][1];
	result.m[1][2] = a.m[1][0] * b.m[0][2] + a.m[1][1] * b.m[1][2] + a.m[1][2] * b.m[2][2] + a.m[1][3] * b.m[3][2];
	result.m[1][3] = a.m[1][0] * b.m[0][3] + a.m[1][1] * b.m[1][3] + a.m[1][2] * b.m[2][3] + a.m[1][3] * b.m[3][3];
	
	result.m[2][0] = a.m[2][0] * b.m[0][0] + a.m[2][1] * b.m[1][0] + a.m[2][2] * b.m[2][0] + a.m[2][3] * b.m[3][0];
	result.m[2][1] = a.m[2][0] * b.m[0][1] + a.m[2][1] * b.m[1][1] + a.m[2][2] * b.m[2][1] + a.m[2][3] * b.m[3][1];
	result.m[2][2] = a.m[2][0] * b.m[0][2] + a.m[2][1] * b.m[1][2] + a.m[2][2] * b.m[2][2] + a.m[2][3] * b.m[3][2];
	result.m[2][3] = a.m[2][0] * b.m[0][3] + a.m[2][1] * b.m[1][3] + a.m[2][2] * b.m[2][3] + a.m[2][3] * b.m[3][3];
	
	result.m[3][0] = a.m[3][0] * b.m[0][0] + a.m[3][1] * b.m[1][0] + a.m[3][2] * b.m[2][0] + a.m[3][3] * b.m[3][0];
	result.m[3][1] = a.m[3][0] * b.m[0][1] + a.m[3][1] * b.m[1][1] + a.m[3][2] * b.m[2][1] + a.m[3][3] * b.m[3][1];
	result.m[3][2] = a.m[3][0] * b.m[0][2] + a.m[3][1] * b.m[1][2] + a.m[3][2] * b.m[2][2] + a.m[3][3] * b.m[3][2];
	result.m[3][3] = a.m[3][0] * b.m[0][3] + a.m[3][1] * b.m[1][3] + a.m[3][2] * b.m[2][3] + a.m[3][3] * b.m[3][3];
	
	return result;
}

mat4f orthographic(float left, float right, float top, float bottom, float near, float far) {
	float dx = right - left;
	float dy = top - bottom;
	float dz = far - near;
	
	float sx = 2.0f / dx;
	float sy = 2.0f / dy;
	float sz = 2.0f / dz;
	
	float tx = -(right + left) / dx;
	float ty = -(top + bottom) / dy;
	float tz = -(far + near)   / dz;
	
	return {
		sx, 0, 0, tx,
		0, sy, 0, ty,
		0, 0, sz, tz,
		0, 0, 0, 1,
	};
}