#include "common.h"

#include "sparkles.h"
#include "sparkles_utils.h"
using namespace Sparkles;

extern TimeInfo global_time;

Mesh* square_mesh;
Mesh* circle_mesh;

struct Example {
	const char* name;
	void(*init_function)();
	void(*frame_function)(float dt);
};

void firework_init();
void firework_frame(float dt);

void waterfall_init();
void waterfall_frame(float dt);

const Example examples[] = {
	{"Fireworks!", firework_init, firework_frame},
	{"Waterfall!", waterfall_init, waterfall_frame},
};

int current_example_index = 1;

bool initialize() {
	
	// Create mesh presets.
	
	{
		// Square 
		
		Vertex vertices[4] = {
			{.position = {-0.5f, -0.5f, 0}, .color = {1, 1, 1, 1}, .uv = {0, 0}},
			{.position = {+0.5f, -0.5f, 0}, .color = {1, 1, 1, 1}, .uv = {1, 0}},
			{.position = {+0.5f, +0.5f, 0}, .color = {1, 1, 1, 1}, .uv = {1, 1}},
			{.position = {-0.5f, +0.5f, 0}, .color = {1, 1, 1, 1}, .uv = {0, 1}},
		};
		
		uint32_t indices[6] = {
			0, 1, 2,
			0, 2, 3,
		};
		
		square_mesh = mesh_create(array_size(vertices), vertices, array_size(indices), indices);
	}
	
	{
		// Circle 
		
		// The circle is just a regular polygon with N sides.
		constexpr int N = 20; // Number of sides/triangles/ outer vertices.
		constexpr int vertex_count = 1 + N; // center + one vertex per side.
		constexpr int index_count = N * 3; // One triangle (center and two vertices) per side.
		
		Vertex vertices[vertex_count];
		uint32_t indices[index_count];
		
		// Center
		vertices[0] = {.position = {0, 0, 0}, .color = {1, 1, 1, 1}, .uv = {0.5, 0.5}};
		
		// Generate one vertex per side
		for (int i = 1; i < vertex_count; i += 1) {
			constexpr float dangle = (2.0f * PI) / (float) N;
			
			float angle = dangle * i;
			float x = 0.5f * cos(angle);
			float y = 0.5f * sin(angle);
			
			vertices[i] = {
				.position = {x, y, 0}, 
				.color = {1, 1, 1, 1}, 
				.uv = {x + 0.5f, y + 0.5f}
			};
		}
		
		// Generate the indices for each triangle 
		for (int t = 0; t <= N; t += 1) {
			indices[3 * t + 0] = 0;
			indices[3 * t + 1] = t + 1;
			indices[3 * t + 2] = t + 2;
		}
		
		indices[index_count - 1] = 1;
		
		circle_mesh = mesh_create(array_size(vertices), vertices, array_size(indices), indices);	
	}

	// Initialize all examples.
	for (int i = 0; i < array_size(examples); i += 1) {
		auto current_example = examples[i];
		current_example.init_function();
	}

	return true;
}

void do_frame() {
	// #temporary: We need to remove these OpenGL specific calls here.
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	int num_examples = array_size(examples);
	
	if (num_examples > 0) {
		if (current_example_index < 0) current_example_index = 0;
		if (current_example_index >= num_examples) current_example_index = num_examples - 1;
		auto current_example = examples[current_example_index];
		current_example.frame_function(global_time.dt);
	}
}


