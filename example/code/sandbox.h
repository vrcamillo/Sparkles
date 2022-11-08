#pragma once

// A bunch of utility headers and macros
#include <inttypes.h> // Use proper type names, such as uint32_t, instead of unsigned long long.
#include <stdio.h> // For printf
#include <string.h> // For memset
#include <math.h>
#include <stdlib.h> // For _fullpath

#define array_size(array) (sizeof(array) / sizeof(array[0]))

// We use GLFW for window management
#include "GLFW/glfw3.h"
extern GLFWwindow* the_window;

// To use our library, just include these headers.
#include "sparkles.h"
#include "sparkles_utils.h"
using namespace Sparkles;

// 
// Now, the actual particle data structures and constants.
//

// We provide a number of common geometries to render our particles, but you could use any shape you like.
constexpr int NUM_MESH_PRESETS = 2;
static const char* mesh_presets_names[NUM_MESH_PRESETS] = {
	"Square",
	"Circle",
};

// We provide a number of common textures to render our particles, but you could use any texture you like.
constexpr int NUM_TEXTURE_PRESETS = 4;
static const char* texture_presets_names[NUM_TEXTURE_PRESETS] = {
	"Blank",
	"Blurry Light",
	"Sharp Light",
	"Sharpest Light",
};

constexpr int max_particles_per_emitter = 5000; // Feel free to tweak this.

// To allow you to save your particle configurations, we provide a very simple serialization system.
// Our sandbox state's raw memory is directly saved into or loaded from a binary file. For this reason, 
// we do not store pointers or dynamic arrays into the structs below. Also, be aware that changing any field in these structs may invalidate our simple serialization format.

constexpr int serialization_version = 1;
constexpr int max_emitter_count = 8;
constexpr int max_emitter_color_count = 16;
constexpr int max_attractor_count = 8;

struct Emitter {
	bool active;
	
	vec2 position;
	
	Range1 emission_interval;
	Range1 particles_per_emission;
	
	int mesh_index;
	int texture_index;
	
	Range2 offset;
	Range2 velocity;
	Range1 size;
	Range1 life;
	
	int32_t color_count;
	vec4 colors[max_emitter_color_count];
	float color_weights[max_emitter_color_count];
	
	Emitter() {}
};

enum class ForceType : uint32_t {
	LINEAR = 0,
	INVERSE = 1,
	INVERSE_SQUARED = 2,
};

static const char* force_type_names[] = {
	"Linear (kr)",
	"Inverse (k/r)",
	"Inverse squared (k/r^2)",
};

struct Attractor {
	bool active;
	
	vec2 position;
	ForceType force_type;
	float radius;
	float factor;
	float magnitude_cap;
	
	Attractor() {}
};

struct Physics {
	vec2 gravity;
	float friction;
	
	int32_t attractor_count;
	Attractor attractors[max_attractor_count];
	
	Physics() {}
};

struct SandboxState {
	uint32_t version;
	
	float space_width;
	float space_height;
	
	Physics physics;
	
	int32_t emitter_count;
	Emitter emitters[max_emitter_count];
	
	SandboxState() {}	
};

void emitter_init(Emitter* emitter);
void attractor_init(Attractor* attractor);
void physics_init(Physics* physics);
void sandbox_state_init(SandboxState* state);

void  immediate_init();
void  immediate_rect(Rect rect, vec4 color = {1, 1, 1, 1});
void  immediate_regular_polygon(vec2 center, float radius, int number_of_sides, vec4 color = {1, 1, 1, 1});
void  immediate_line(vec2 a, vec2 b, float line_width, vec4 color = {1, 1, 1, 1});
void  immediate_arrow_head(vec2 position, vec2 direction, float radius, vec4 color = {1, 1, 1, 1});
Mesh* immediate_mesh();
void  immediate_flush(RenderState* state);

bool sandbox_state_load(SandboxState* state, const char* file_path);
void sandbox_state_save(SandboxState* state, const char* file_path);