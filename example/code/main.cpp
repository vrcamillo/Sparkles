#include "common.h"

#include "sparkles.h"
#include "sparkles_utils.h"
using namespace Sparkles;

extern TimeInfo global_time;

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

#include <direct.h> // #hack for mkdir

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
	"Blurry Light",
	"Sharp Light",
	"Sharpest Light",
};

RenderTarget* hdr_render_target;
RenderState hdr_blit_render_state;

struct RecordState {
	bool recording;
	int frames_to_record = 60;
	int frame_index;
	
	uint32_t* frames_data;
	
	char name[128];
};

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
	{"Sandbox!", sandbox_init, sandbox_frame},
	{"Bomb!", bomb_init, bomb_frame},
};

int current_example_index = 0;

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
		texture_presets[0] = nullptr;
		texture_presets[1] = texture_generate_light_mask(64, 64, 0.01, 1);
		texture_presets[2] = texture_generate_light_mask(64, 64, 0.01, 5);
		texture_presets[3] = texture_generate_light_mask(64, 64, 0.01, 10);
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
	
	using namespace ImGui;
	
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
	
	{
		static RecordState record_state;
		
		int width = hdr_render_target->width;
		int height = hdr_render_target->height;
		
		SetNextWindowPos(ImVec2(display_w * 0.5, 0), 0, ImVec2(0.5, 0));
		if (Begin("Record", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize)) {				
			ImGui::BeginDisabled(record_state.recording);
			if (Button("Record")) OpenPopup("Record?");
			ImGui::EndDisabled();
			
			ImGui::BeginDisabled(!record_state.recording);
			SameLine();
			if (Button("Cancel")) record_state.recording = false;
			ImGui::EndDisabled();
			
			if (record_state.recording) {
				Text("Recording frame %d of %d...", record_state.frame_index, record_state.frames_to_record);
			}
			
			ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (BeginPopupModal("Record?", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
				Text("Number of frames");
				SameLine();
				SetNextItemWidth(100);
				InputInt("##record_frames", &record_state.frames_to_record, 30);

				Text("Save as...");
				SameLine();
				SetNextItemWidth(100);
				InputText("##record_name", record_state.name, sizeof(record_state.name));
				
				BeginDisabled(!strlen(record_state.name) || record_state.frames_to_record <= 0);
				if (Button("Start", ImVec2(100, 0))) {
					record_state.recording = true;
					record_state.frame_index = 0;
					
					record_state.frames_data = new uint32_t[width * height * record_state.frames_to_record];
					
					ImGui::CloseCurrentPopup();
				}
				EndDisabled();
				
				SameLine();
				
				if (Button("Cancel", ImVec2(100, 0))) {
					ImGui::CloseCurrentPopup();
				}
				
				ImGui::EndPopup();
			}
			
			ImGui::End();
		}
		
		if (record_state.recording && record_state.frame_index >= record_state.frames_to_record) {
			record_state.recording = false;
			
			mkdir(record_state.name);
			
			int small_width = width / 2;
			int small_height = height / 2;
			uint32_t* smaller_frame_data = new uint32_t[small_width * small_height];
			
			for (int i = 0; i < record_state.frames_to_record; i += 1) {
				uint32_t* frame_data = record_state.frames_data + (width * height * i);
				
				stbir_resize_uint8((uint8_t*)frame_data, width, height, 0, (uint8_t*)smaller_frame_data, small_width, small_height, 0, 4);
				
				char filename[256];
				sprintf_s(filename, "%s/%03d.png", record_state.name, i);
				stbi_flip_vertically_on_write(1);
				stbi_write_png(filename, small_width, small_height, 4, smaller_frame_data, 0);
			}
			
			delete[] smaller_frame_data;
			delete[] record_state.frames_data;
			record_state.frames_data = nullptr;
		}
		
		if (record_state.recording) {
			uint32_t* frame_data = record_state.frames_data + (width * height * record_state.frame_index);
			glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, frame_data);
			record_state.frame_index += 1;
		}
		
	}
}