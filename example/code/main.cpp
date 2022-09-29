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

int current_example_index = 0;

bool initialize() {
	Sparkles::initialize();
	
	{
		// Create mesh presets.
		square_mesh = mesh_generate_quad({-0.5, -0.5}, {+0.5, +0.5});
		circle_mesh = mesh_generate_regular_polygon(20);
	}
	
	// Initialize all examples.
	for (int i = 0; i < array_size(examples); i += 1) {
		auto current_example = examples[i];
		current_example.init_function();
	}

	return true;
}

void do_frame() {
	render_target_clear(nullptr, {0, 0, 0, 1});	
	int num_examples = array_size(examples);
	
	if (num_examples > 0) {
		if (current_example_index < 0) current_example_index = 0;
		if (current_example_index >= num_examples) current_example_index = num_examples - 1;
		auto current_example = examples[current_example_index];
		current_example.frame_function(global_time.dt);
	}
}

void ImGui::RandomEdit(const char* label, Sparkles::RandomScalar* scalar, float min, float max) {
	ImGui::Text(label);
	ImGui::Indent();
	ImGui::PushID(scalar);
	ImGui::SliderFloat("Min", &scalar->min, min, max);
	ImGui::SliderFloat("Max", &scalar->max, min, max);
	ImGui::PopID();
	ImGui::Unindent();
}


void ImGui::RandomEdit(const char* label, Sparkles::RandomColor* color) {	
	vec4 color_min = {color->red.min, color->green.min, color->blue.min, color->alpha.min};
	vec4 color_max = {color->red.max, color->green.max, color->blue.max, color->alpha.max};	
	
	ImGui::Text(label);
	ImGui::Indent();
	
	ImGui::PushID(color);
	ImGui::ColorEdit4("Min", &color_min.x, ImGuiColorEditFlags_DisplayRGB);
	// ImGui::SameLine();
	ImGui::ColorEdit4("Max", &color_max.x, ImGuiColorEditFlags_DisplayRGB);
	ImGui::PopID();
	ImGui::Unindent();
	
	*color = uniform_rgba(color_min, color_max);	
}
