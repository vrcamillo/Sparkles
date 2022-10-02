#include "common.h"

#include "sparkles.h"
#include "sparkles_utils.h"
using namespace Sparkles;

extern TimeInfo global_time;

Mesh* square_mesh;
Mesh* circle_mesh;
Mesh* mesh_presets[NUM_MESH_PRESETS];
const char* mesh_presets_names[NUM_MESH_PRESETS] = {
	"Square",
	"Circle",
};

Texture* light_texture;
Texture* texture_presets[NUM_TEXTURE_PRESETS];
const char* texture_presets_names[NUM_TEXTURE_PRESETS] = {
	"Blank",
	"Radial Light (64x64)",
};

RenderTarget* hdr_render_target;
RenderState hdr_blit_render_state;

struct Example {
	const char* name;
	void(*init_function)();
	void(*frame_function)(float dt);
};

void firework_init();
void firework_frame(float dt);

void bomb_init();
void bomb_frame(float dt);

void sandbox_init();
void sandbox_frame(float dt);

const Example examples[] = {
	{"Fireworks!", firework_init, firework_frame},
	{"Bomb!", bomb_init, bomb_frame},
	{"Sandbox!", sandbox_init, sandbox_frame},
};

int current_example_index = 2;

bool initialize() {
	Sparkles::initialize();
	
	{
		//
		// Initialize our graphics variables.
		//
		
		// Create our HDR render target the same size as our backbuffer.
		int backbuffer_width, backbuffer_height;
		glfwGetFramebufferSize(global_window, &backbuffer_width, &backbuffer_height);
		hdr_render_target = render_target_create(TextureFormat::RGBA_FLOAT16, backbuffer_width, backbuffer_height);
		
		hdr_blit_render_state.projection = orthographic(-0.5f, +0.5f, +0.5f, -0.5f, -1, +1);;
		hdr_blit_render_state.viewport = {0, 0, (float) backbuffer_width, (float) backbuffer_height};
		hdr_blit_render_state.texture0 = nullptr; // Will be our HDR texture.
	}
	
	{
		// Create mesh presets.
		square_mesh = mesh_generate_quad({-0.5, -0.5}, {+0.5, +0.5});
		circle_mesh = mesh_generate_regular_polygon(20);
		mesh_presets[0] = square_mesh;
		mesh_presets[1] = circle_mesh;
	}
	
	{
		// Create texture presets.
		light_texture = texture_generate_light_mask(64, 64, 0.01, 10);
		texture_presets[0] = nullptr;
		texture_presets[1] = light_texture;
	}
	
	// Initialize all examples.
	for (int i = 0; i < array_size(examples); i += 1) {
		auto current_example = examples[i];
		current_example.init_function();
	}

	return true;
}

int starvation_count;
float starvation_timer = 0;
void notify_starvation(int count) {
	starvation_count += count;
	if (starvation_count) starvation_timer = 1;
}

void do_frame() {
	render_target_clear(nullptr, {0, 0, 0, 1});	
	int num_examples = array_size(examples);
	
	starvation_count = 0;
	
	if (num_examples > 0) {
		if (current_example_index < 0) current_example_index = 0;
		if (current_example_index >= num_examples) current_example_index = num_examples - 1;
		auto current_example = examples[current_example_index];
		current_example.frame_function(global_time.dt);
		
		hdr_blit_render_state.texture0 = render_target_flush(hdr_render_target);
		mesh_render(square_mesh, &hdr_blit_render_state);
	}
	
	int display_w, display_h;
	glfwGetFramebufferSize(global_window, &display_w, &display_h);
	ImGui::SetNextWindowPos(ImVec2(display_w, 0), 0, ImVec2(1, 0));
	ImGui::Begin("Info", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
	ImGui::Text("FPS: %.0f", (1.0f / global_time.dt));
	if (starvation_timer > 0) {
		ImGui::TextColored(ImVec4(0.9, 0.1, 0.1, 1), "There are too many particles! \nDecrease the emission rate!");
	}
	
	starvation_timer -= global_time.dt;
	starvation_timer = fmax(starvation_timer, 0);
	ImGui::End();
}