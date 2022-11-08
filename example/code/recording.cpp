#include <direct.h> // #temporary #hack for mkdir

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

// Disabled temporarily while the UI refactor is not done.

#if 0
struct RecordState {
	bool recording;
	int frames_to_record = 60;
	int frame_index;
	
	uint32_t* frames_data;
	
	char name[128];
};

void do_record_ui() {
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
#endif