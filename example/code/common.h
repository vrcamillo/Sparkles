#pragma once 

#include <inttypes.h> // Use proper type names, such as uint32_t, instead of unsigned long long.
#include <stdio.h> // For printf
#include <math.h>

#include "GLFW/glfw3.h"
#include "imgui.h"

#define array_size(array) sizeof(array) / sizeof((array)[0])

struct TimeInfo {
	float max_dt = 0; // The maximum allowed frame time in seconds; set at initialization time.
	float min_dt = 0; // The minimum allowed frame time in seconds; set at initialization time.
	float dt = 0; // The actual frame time in seconds; set every loop. 
};

extern GLFWwindow* global_window;
// extern TimeInfo global_time;

#include "sparkles.h"
#include "sparkles_utils.h"

extern Sparkles::Mesh* immediate_mesh;
extern Sparkles::MeshBuilder immediate_builder;

extern Sparkles::RenderTarget* hdr_render_target;
extern Sparkles::RenderState hdr_blit_render_state;

constexpr int NUM_MESH_PRESETS = 2;
extern Sparkles::Mesh* circle_mesh;
extern Sparkles::Mesh* square_mesh;
extern Sparkles::Mesh* mesh_presets[NUM_MESH_PRESETS];
extern const char* mesh_presets_names[NUM_MESH_PRESETS];

constexpr int NUM_TEXTURE_PRESETS = 3 + 1; // number of textures + 1 null one.
extern Sparkles::Texture* texture_presets[NUM_TEXTURE_PRESETS];
extern const char* texture_presets_names[NUM_TEXTURE_PRESETS];

void notify_starvation(int count);

// #temporary: Remove these.
// #define min(a, b) (a < b) ? (a) : (b)
// #define max(a, b) (a > b) ? (a) : (b)
