#include "math.h"

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