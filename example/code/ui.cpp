#include "sandbox.h"

// We use ImGui for controlling particle simulation variables and visuals.
#include "imgui.h"
using namespace ImGui;

char dropped_path[1024];
bool path_just_dropped = false;

// SandboxState* the_state;

// Some utility functions
template <typename T>
void array_ordered_remove(int* array_count, T array[], int index_to_remove) {
	for (int i = index_to_remove; i < *array_count - 1; i += 1) {
		array[i] = array[i + 1];
	}
	*array_count -= 1;
}

constexpr ImVec4 add_color = ImVec4(0.1, 0.5, 0.1, 1);
constexpr ImVec4 delete_color = ImVec4(0.8, 0.1, 0.1, 1);

void DragAngleRange2(const char* label, float* min_radians, float* max_radians, float speed = 1, float min_degrees = 0, float max_degrees = 360) {
	float min_value_degrees = *min_radians * (360.0f / TAU);
	float max_value_degrees = *max_radians * (360.0f / TAU);
	ImGui::DragFloatRange2(label, &min_value_degrees, &max_value_degrees, speed, min_degrees, max_degrees, "Min: %.0f deg", "Max: %.0f deg", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SameLine(); if (ImGui::Button("All")) { min_value_degrees = 0; max_value_degrees = 360;}
	*min_radians = min_value_degrees * (TAU / 360.0);
	*max_radians = max_value_degrees * (TAU / 360.0);
}

bool ColoredButton(ImVec4 color, const char* label) {
	ImGui::PushStyleColor(ImGuiCol_Button, color);
	bool result = ImGui::Button(label);
	ImGui::PopStyleColor();
	return result;
}

bool sandbox_state_load(SandboxState* state, const char* file_path) {
	bool result = false;
	
	auto file = fopen(file_path, "rb");
	if (file) {
		fseek(file, 0, SEEK_END);
		auto size = ftell(file);
		fseek(file, 0, SEEK_SET);
		
		if (size == sizeof(SandboxState)) {
			fread(state, sizeof(SandboxState), 1, file);
			result = true;
		}
		fclose(file);
	}
	
	return result;
}

void drop_callback(GLFWwindow* window, int count, const char** paths) {
	strcpy_s(dropped_path, paths[0]);
	path_just_dropped = true;
}

void sandbox_ui(SandboxState* state) {
	// the_state = state;
	
	glfwSetDropCallback(the_window, drop_callback);
	
	//
	// Menu bar
	//
	
	bool try_to_reset = false;
	bool try_to_load = false;
	bool try_to_save = false;
	
	if (BeginMainMenuBar()) {
		if (BeginMenu("State")) {
			try_to_reset = MenuItem("Reset", "Ctrl+N");
			try_to_load  = MenuItem("Load", "Ctrl+O");
			try_to_save  = MenuItem("Save", "Ctrl+S");
			
			Separator();
			
			if (MenuItem("Record")) {
				
			}
			EndMenu();
		}
		
		// if (BeginMenu("View")) {
		// MenuItem("Emitter editor", nullptr, &emitter_editor);
		// EndMenu();
		// }
		
		EndMainMenuBar();
	}
	
	if (try_to_reset) {
		sandbox_state_init(state);
	}
	
	if (try_to_load) {
		OpenPopup("Load state...");
	}
	
	ImVec2 center = GetMainViewport()->GetCenter();
	SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (BeginPopupModal("Load state...", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
		Text("Drag and drop any .sparkles into this window to load.");

		if (path_just_dropped) {
			CloseCurrentPopup();
			
			if (!sandbox_state_load(state, dropped_path)) {
				// #robustness: Show error message
			}
		}
		
		EndPopup();
	}
	path_just_dropped = false;
}

#if 0
void sandbox_panel() {
	SetNextWindowPos(ImVec2(0, 0));
	Begin("Sandbox!", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	
	if (Button("Save state")) {
		OpenPopup("Save state as...");
	}
	
	ImVec2 center = GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (BeginPopupModal("Save state as...", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		
		static char name[128];
		
		InputTextWithHint("##state_name", "Enter the file name here", name, sizeof(name));
		
		BeginDisabled(!strlen(name));
		if (Button("Save")) {
			char filename[256];
			sprintf_s(filename, "%s.sparkles", name);
			
			auto file = fopen(filename, "wb");
			if (file) {
				fwrite(&state, 1, sizeof(state), file);
			}
			fclose(file);
			
			ImGui::CloseCurrentPopup();
		}
		EndDisabled();
		
		SameLine();
		
		if (Button("Cancel")) {
			ImGui::CloseCurrentPopup();
		}
		
		EndPopup();
	}
	
	// Checkbox("Debug visualization", &debug_view);
	
	if (TreeNodeEx("Physics", ImGuiTreeNodeFlags_DefaultOpen)) {
		Physics* physics = &state.physics;
		
		static float gravity_angle;
		static float gravity_magnitude;
		
		gravity_angle = atan2(physics->gravity.y, physics->gravity.x);
		gravity_magnitude = norm(physics->gravity);
		
		SliderAngle("Gravity angle", &gravity_angle, -180, 180);
		SameLine(); if (Button("^")) gravity_angle = TAU * 0.25f;
		SameLine(); if (Button("v")) gravity_angle = -TAU * 0.25f;
		DragFloat("Gravity magnitude", &gravity_magnitude, 0.1, 0.001, 20, "%.1f"); 		
		physics->gravity = polar(gravity_magnitude, gravity_angle);
		
		SliderFloat("Friction factor", &physics->friction, 0.8, 1, "%.3f", ImGuiSliderFlags_Logarithmic);
		
		int delete_attractor_index = -1;
		for (int i = 0; i < physics->attractor_count; i += 1) {
			Attractor* attractor = &physics->attractors[i];
			
			PushID(i);
			
			if (TreeNodeEx("Attractor", ImGuiTreeNodeFlags_DefaultOpen, "Attractor %d", i + 1)) {
				
				Checkbox("Active", &attractor->active);
				SameLine();
				if (ColoredButton(delete_color, "Delete")) delete_attractor_index = i;
				
				DragFloat2("Position (x, y)", &attractor->position.x, 0.05, -10, +10, "%.1f");
				DragFloat("Influence radius", &attractor->radius, 0.05, 0, +20, "%.1f");
				
				Combo("Force type", (int*) &attractor->force_type, force_type_names, array_size(force_type_names));
				DragFloat("Factor", &attractor->factor, 0.1, -100, +100, "%.1f");
				DragFloat("Magnitude cap", &attractor->magnitude_cap, 0.1, 0, 100, "%.1f");
				TreePop();
			}
			PopID();
		}
		
		if (physics->attractor_count < array_size(physics->attractors)) {
			
			if (ColoredButton(add_color, "+ New attractor")) {
				if (physics->attractor_count > 0) {
					physics->attractors[physics->attractor_count] = physics->attractors[physics->attractor_count - 1];
				} else {
					attractor_init(&physics->attractors[physics->attractor_count]);
				}
				physics->attractor_count += 1;
			}
		}
		
		if (delete_attractor_index >= 0) array_ordered_remove(&physics->attractor_count, physics->attractors, delete_attractor_index);
		
		TreePop();
	}
	
	int delete_emitter_index = -1;
	for (int s = 0; s < state.emitter_count; s += 1) {
		auto emitter = &state.emitters[s];
		
		PushID(s);
		
		if (TreeNodeEx("Emitter", ImGuiTreeNodeFlags_DefaultOpen, "Emitter %d", s + 1)) {
			Checkbox("Active", &emitter->active);
			
			if (s != 0) {
				SameLine();
				if (ColoredButton(delete_color, "Delete")) delete_emitter_index = s;
			}
			
			ImGui::BulletText("Simulation");
			
			DragFloat2("Position (x, y)", &emitter->position.x, 0.05, -10, +10, "%.1f");
			
			DragFloatRange2("Emission period", &emitter->emission_interval.min, &emitter->emission_interval.max, 0.01, 0.05, 10, "min = %.2f s", "max = %.2f s", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_Logarithmic);
			DragFloatRange2("Particles per emission", &emitter->particles_per_emission.min, &emitter->particles_per_emission.max, 0.5, 0, max_particles_per_emitter, "min = %.0f", "max = %.0f", ImGuiSliderFlags_AlwaysClamp);
			
			ImGui::BulletText("Visuals");
			
			Combo("Shape", &emitter->mesh_index, mesh_presets_names, NUM_MESH_PRESETS); 
			Combo("Texture", &emitter->texture_index, texture_presets_names, NUM_TEXTURE_PRESETS); 
			
			int delete_color_index = -1;
			for (int i = 0; i < emitter->color_count; i += 1) {
				PushID(i);
				
				ColorEdit4("##palette", (float*) &emitter->colors[i], ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoInputs);
				SameLine();
				Text("Color %d", i + 1);
				
				SameLine();
				SetNextItemWidth(100); 
				DragFloat("", &emitter->color_weights[i], 0.05, 0, 5, "weight = %.1f");
				
				if (i != 0) {
					SameLine();
					if (ColoredButton(delete_color, " - ")) delete_color_index = i;
				}
				
				PopID();
			}
			
			if (emitter->color_count < array_size(emitter->colors)) {
				PushStyleColor(ImGuiCol_Button, add_color);
				if (Button("+ New color")) {
					emitter->colors[emitter->color_count] = emitter->colors[emitter->color_count - 1];
					emitter->color_weights[emitter->color_count] = 1;
					emitter->color_count += 1;
				}
				PopStyleColor();
			}
			
			if (delete_color_index >= 0) {
				array_ordered_remove(&emitter->color_count, emitter->colors, delete_color_index);	
				emitter->color_count += 1; // #hack
				array_ordered_remove(&emitter->color_count, emitter->color_weights, delete_color_index);
			}
			
			ImGui::BulletText("Particle");
			
			static_assert(sizeof(emitter->offset.coords) == sizeof(int), "We assume coords is 4 bytes long, for ImGui reasons. If that should not be the case anymore, change the calls to RadioButton below.");
			Text("Offset coords: ");
			SameLine();
			RadioButton("Cartesian", (int*) &emitter->offset.coords, (int) Coords2D::CARTESIAN);
			SameLine();
			RadioButton("Polar", (int*) &emitter->offset.coords, (int) Coords2D::POLAR);
			
			switch (emitter->offset.coords) {
			  case Coords2D::CARTESIAN: 
				DragFloatRange2("X offset", &emitter->offset.min.x, &emitter->offset.max.x, 0.1, -10, +10, "Min: %.1f", "Max: %.1f", ImGuiSliderFlags_AlwaysClamp);
				DragFloatRange2("Y offset", &emitter->offset.min.y, &emitter->offset.max.y, 0.1, -10, +10, "Min: %.1f", "Max: %.1f", ImGuiSliderFlags_AlwaysClamp);
				break;
			  case Coords2D::POLAR:
				DragFloatRange2("Radius offset", &emitter->offset.min.x, &emitter->offset.max.x, 0.1, 0, 10, "Min: %.1f", "Max: %.1f", ImGuiSliderFlags_AlwaysClamp);
				DragAngleRange2("Angle offset", &emitter->offset.min.y, &emitter->offset.max.y);
				
			  default: break;
			}
			
			DragFloatRange2("Speed", &emitter->velocity.min.x, &emitter->velocity.max.x, 0.1, 0, 20, "Min: %.1f", "Max: %.1f", ImGuiSliderFlags_AlwaysClamp);
			DragAngleRange2("Direction", &emitter->velocity.min.y, &emitter->velocity.max.y);
			DragFloatRange2("Size", &emitter->size.min, &emitter->size.max, 0.01, 0, 4, "min = %.2f", "max = %.2f", ImGuiSliderFlags_AlwaysClamp);
			DragFloatRange2("Life", &emitter->life.min, &emitter->life.max, 0.05, 0, 20, "min = %.1f s", "max = %.1f s", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_Logarithmic);
			
			TreePop();	
		}
		
		PopID();
	}
	
	if (state.emitter_count < max_emitter_count) {
		if (ColoredButton(add_color, "+ New emitter")) {
			state.emitters[state.emitter_count] = state.emitters[state.emitter_count - 1];
			state.emitter_count += 1;
		}
	}
	
	if (delete_emitter_index >= 0) array_ordered_remove(&state.emitter_count, state.emitters, delete_emitter_index);
	
	End();
}
#endif