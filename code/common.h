#pragma once 

#include <inttypes.h> // Use proper type names, such as uint32_t, instead of unsigned long long.
#include <stdio.h>

#include "glad/gl.h"
#include "GLFW/glfw3.h"

#include "my_math.h"

void system_sleep_ms(int milliseconds);

struct TimeInfo {
	float max_dt = 0; // The maximum allowed frame time in seconds; set at initialization time.
	float min_dt = 0; // The minimum allowed frame time in seconds; set at initialization time.
	float dt = 0; // The actual frame time in seconds; set every loop. 
};

extern GLFWwindow* global_window;
extern TimeInfo global_time;

