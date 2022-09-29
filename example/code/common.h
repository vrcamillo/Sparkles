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

namespace Sparkles {
	struct Mesh;
	struct RandomScalar;
	struct RandomColor;
}
extern Sparkles::Mesh* square_mesh;
extern Sparkles::Mesh* circle_mesh;

// #temporary: Remove these.
// #define min(a, b) (a < b) ? (a) : (b)
// #define max(a, b) (a > b) ? (a) : (b)

namespace ImGui {
	void RandomEdit(const char* label, Sparkles::RandomScalar* scalar, float min, float max);
	void RandomEdit(const char* label, Sparkles::RandomColor* color);
}