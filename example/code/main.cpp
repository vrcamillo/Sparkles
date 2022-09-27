#include "common.h"

using namespace Sparkles;

Mesh* square_mesh;
Mesh* circle_mesh;

void firework_initialize();
void firework_frame();

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
	
	firework_initialize();
	return true;
}

void do_frame() {
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	firework_frame();
}

mat4 orthographic(float left, float right, float top, float bottom, float near, float far) {
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


