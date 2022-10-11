#include "sandbox.h"

static MeshBuilder the_builder;
static Mesh* the_mesh;

void immediate_init() {
	the_builder = mesh_builder_create(1024, 1024 * 3);
	the_mesh = mesh_create(the_builder.vertex_capacity, the_builder.index_capacity);
}

void immediate_rect(Rect rect) {		
	float x0 = rect.x;
	float x1 = rect.x + rect.w;
	float y0 = rect.y;
	float y1 = rect.y + rect.h;
	
	auto vertices = put_vertices(&the_builder, 4);
	vertices[0] = {x0, y0, 0, 1, 1, 1, 1, 0, 0};
	vertices[1] = {x1, y0, 0, 1, 1, 1, 1, 1, 0};
	vertices[2] = {x1, y1, 0, 1, 1, 1, 1, 1, 1};
	vertices[3] = {x0, y1, 0, 1, 1, 1, 1, 0, 1};
	
	put_indices(&the_builder, &vertices[0], 0, 1, 2);
	put_indices(&the_builder, &vertices[0], 0, 2, 3);
}

void immediate_regular_polygon(vec2 center, float radius, int number_of_sides) {
	int vertex_count = 1 + number_of_sides; // center + one vertex per side.
	int index_count = number_of_sides * 3; // One triangle (center and two vertices) per side.
	
	auto vertices = put_vertices(&the_builder, vertex_count);
	
	// Center
	vertices[0] = {
		center.x, center.y, 0, 
		1, 1, 1, 1, 
		0.5, 0.5
	};
	
	float dangle = (2.0f * PI) / (float) number_of_sides;
	
	// Generate one vertex per side
	for (int i = 1; i < vertex_count; i += 1) {
		
		float angle = dangle * i;
		float x = cos(angle);
		float y = sin(angle);
		
		vertices[i] = {
			center.x + x * radius, center.y + y * radius, 0, 
			1, 1, 1, 1, 
			0.5f * x + 0.5f, 0.5f * y + 0.5f
		};
	}
	
	// Generate the indices for each triangle 
	for (int t = 0; t < number_of_sides; t += 1) {
		put_indices(&the_builder, &vertices[0], 0, t + 1, t + 2);
	}
	
	put_indices(&the_builder, &vertices[0], 0, number_of_sides, 1);
}

void immediate_bezier(CubicBezier* curve, float line_width, int number_of_points) {
	float dt = 1.0f / number_of_points;
	for (int i = 0; i < number_of_points; i += 1) {			
		float t = dt * i;
		vec2 center = bezier_get_position(*curve, t);
		vec2 tangent = normalize(bezier_get_tangent(*curve, t));
		vec2 offset = (line_width * 0.5f) * rotate2(tangent, TAU * 0.25);
		
		vec2 p0 = center + offset;
		vec2 p1 = center - offset;
		
		auto vertices = put_vertices(&the_builder, 2);
		vertices[0] = {p0.x, p0.y, 0, 1, 1, 1, 1, 0, 0};		
		vertices[1] = {p1.x, p1.y, 0, 1, 1, 1, 1, 0, 0};
		
		if (i < number_of_points - 1) {
			put_indices(&the_builder, vertices, 0, 1, 2);
			put_indices(&the_builder, vertices, 1, 2, 3);
		}
	}	
}

Mesh* immediate_mesh() {
	auto mesh = mesh_create(the_builder.vertex_cursor, the_builder.index_cursor, the_builder.vertices, the_builder.indices);
	mesh_builder_clear(&the_builder);
	return mesh;
}

void immediate_flush(RenderState* state) {
	mesh_upload(the_mesh, the_builder.vertex_cursor, the_builder.vertices, the_builder.index_cursor, the_builder.indices);
	mesh_render(the_mesh, state, the_builder.index_cursor);	
	mesh_builder_clear(&the_builder);
}
