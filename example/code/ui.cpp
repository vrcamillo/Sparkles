#include "sandbox.h"

// #todo: We would like a better visualization tool for polar and rectangular vectors than 1d sliders.

// We use ImGui for controlling particle simulation variables and visuals.
#define IMGUI_USER_CONFIG "my_imgui_config.h"
#include "imgui.h"
#include "imgui_internal.h"
using namespace ImGui;

// These are global variables used by the UI. 
// Eventually we may group them in a single struct.

char dropped_path[1024];
char name_to_save[128];
bool path_just_dropped = false;

bool load_dialog_open = false;
bool save_dialog_open = false;

bool draw_debug_view = false;
bool draw_grid = true;
bool draw_positions = true;
bool draw_force_fields = true;

int starvation_count;
float starvation_timer = 0;

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


vec2 world_to_screen(SandboxState* state, vec2 world_position) {
	vec2 screen_size = GetMainViewport()->Size;
	vec2 world_size = vec2(state->space_width, state->space_height);
	vec2 normalized_device_position = world_position / world_size;
	vec2 screen_position = ((normalized_device_position + vec2(1, 1)) / 2.0f) * screen_size;
	return screen_position;
}

vec2 screen_to_world(SandboxState* state, vec2 screen_position) {
	vec2 screen_size = GetMainViewport()->Size;
	vec2 world_size = vec2(state->space_width, state->space_height);
	vec2 normalized_device_position = (vec2(screen_position.x, screen_size.y - screen_position.y) - screen_size * 0.5f) / screen_size;
	vec2 world_position = normalized_device_position * world_size;
	return world_position;
}

ImGuiID world_active_item;

void world_drag_point(SandboxState* state, vec2* position, float radius = 0.1, vec4 color = {1, 1, 1, 1}) {	
	vec2 mouse_in_world = screen_to_world(state, GetMousePos());
	
	PushID("WorldDragPoint");
	
	// auto window = GetCurrentWindow();
	auto id = GetID(position);
	
	if (norm(mouse_in_world - *position) <= radius) {
		radius *= 1.2;
		
		if (!world_active_item && !IsAnyItemActive() && !IsAnyItemHovered() && IsMouseClicked(ImGuiMouseButton_Left)) {
			world_active_item = id;
		}
	}
	
	if (world_active_item == id) {
		if (!IsMouseDown(ImGuiMouseButton_Left)) {
			world_active_item = 0;
		}
		
		if (IsMouseDragging(ImGuiMouseButton_Left)) {
			*position = mouse_in_world;
		}
	}
	
	PopID();
	
	immediate_regular_polygon(*position, radius, 20, color);
}

void notify_starvation(int count) {
	starvation_count += count;
	if (starvation_count) starvation_timer = 1;
}

void drop_callback(GLFWwindow* window, int count, const char** paths) {
	strcpy_s(dropped_path, paths[0]);
	path_just_dropped = true;
}

void sandbox_ui(SandboxState* state, float dt) {	
	glfwSetDropCallback(the_window, drop_callback); // Used for loading files.
	
	//
	// Menu bar
	//
	
	// Due to the way ImGui works, we need to open popups outside of BeginMainMenuBar() and EndMainMenuBar().
	// So, we need these variables.
	bool try_to_reset = false;
	bool try_to_load = false;
	bool try_to_save = false;
	
	ImVec2 menu_bar_size;
	
	if (BeginMainMenuBar()) {
		menu_bar_size = GetWindowSize();
		
		if (BeginMenu("State")) {
			try_to_reset = MenuItem("Reset", "Ctrl+N");
			try_to_load  = MenuItem("Load", "Ctrl+O");
			try_to_save  = MenuItem("Save", "Ctrl+S");
			
#if 0 // Disabled for now.
			Separator();
			
			if (MenuItem("Record")) {
				
			}
#endif
			EndMenu();
		}
		
		if (BeginMenu("Debug view")) {
			MenuItem("Enabled", nullptr, &draw_debug_view);
			Separator();
			MenuItem("Grid", nullptr, &draw_grid, draw_debug_view);
			MenuItem("Positions", nullptr, &draw_positions, draw_debug_view);
			EndMenu();
		}
		
		EndMainMenuBar();
	}
	
	if (try_to_reset) {
		sandbox_state_init(state);
	}
	
	if (try_to_load) {
		load_dialog_open = true;
		OpenPopup("Load state...");
	}
	
	if (try_to_save) {
		save_dialog_open = true;
		memset(name_to_save, 0, sizeof(name_to_save));
		OpenPopup("Save state...");
	}
	
	ImVec2 center = GetMainViewport()->GetCenter();
	
	//
	// Load dialog
	//
	SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (BeginPopupModal("Load state...", &load_dialog_open, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
		Text("Drag and drop any .sparkles into this window to load.");

		if (path_just_dropped) {
			CloseCurrentPopup();
			sandbox_state_load(state, dropped_path);
		}
		
		if (IsKeyPressed(ImGuiKey_Escape)) CloseCurrentPopup();
		
		EndPopup();
	}
	path_just_dropped = false;
	
	//
	// Save dialog
	//
	SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (BeginPopupModal("Save state...", &save_dialog_open, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
				
		bool should_save = false;
		
		if (!IsAnyItemActive()) SetKeyboardFocusHere();
		should_save = InputTextWithHint("##state_name", "Enter save name", name_to_save, sizeof(name_to_save), ImGuiInputTextFlags_EnterReturnsTrue);
		
		char path[1024];
		sprintf_s(path, "%s.sparkles", name_to_save);
		
		char absolute_path[1024];
		_fullpath(absolute_path, path, sizeof(absolute_path));
		
		Text("Output will be written to: ");
		Text("\t%s", absolute_path); // #todo: write absolute path here.
		
		if (should_save) {
			CloseCurrentPopup();
			sandbox_state_save(state, absolute_path);
		}
		
		if (IsKeyPressed(ImGuiKey_Escape)) CloseCurrentPopup();
		
		EndPopup();
	}
	
	//
	// Simulation panel
	//
	{
		Begin("Sandbox!", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		if (TreeNodeEx("Physics", ImGuiTreeNodeFlags_DefaultOpen)) {
			Physics* physics = &state->physics;
			
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
		for (int s = 0; s < state->emitter_count; s += 1) {
			auto emitter = &state->emitters[s];
			
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
		
		if (state->emitter_count < max_emitter_count) {
			if (ColoredButton(add_color, "+ New emitter")) {
				state->emitters[state->emitter_count] = state->emitters[state->emitter_count - 1];
				state->emitter_count += 1;
			}
		}
		
		if (delete_emitter_index >= 0) array_ordered_remove(&state->emitter_count, state->emitters, delete_emitter_index);
		
		End();
	}
	
	//
	// Info panel
	// 
	{
		SetNextWindowPos(ImVec2(GetMainViewport()->Size.x, menu_bar_size.y), 0, ImVec2(1, 0));
		Begin("Info", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
		Text("FPS: %.0f", (1.0f / dt));
		if (starvation_timer > 0) {
			TextColored(ImVec4(0.9, 0.1, 0.1, 1), "There are too many particles! \nDecrease the emission rate!");
		}
		
		starvation_timer -= dt;
		starvation_timer = fmax(starvation_timer, 0);
		starvation_count = 0;
		End();
	}
	

	if (draw_debug_view) {
		int backbuffer_width, backbuffer_height;
		glfwGetFramebufferSize(the_window, &backbuffer_width, &backbuffer_height);
		
		RenderState debug_render_state = {};
		debug_render_state.projection = orthographic(-state->space_width * 0.5, +state->space_width * 0.5, +state->space_height * 0.5, -state->space_height * 0.5, -1, +1);
		debug_render_state.viewport = {0, 0, (float) backbuffer_width, (float) backbuffer_height};
		
		if (draw_grid) {
			constexpr vec4 grid_color = {1, 1, 1, 0.1};
			constexpr float grid_width = 0.01f;
			
			auto x0 = -state->space_width * 0.5f;
			auto x1 = +state->space_width * 0.5f;
			auto y0 = -state->space_height * 0.5f;
			auto y1 = +state->space_height * 0.5f;
			
			auto dx = (x1 - x0) / state->space_width;
			for (int i = 0; i <= (int) state->space_width; i += 1) {
				float x = x0 + dx * i;
				immediate_line({x, y0}, {x, y1}, grid_width, grid_color);
			}
			
			auto dy = (y1 - y0) / state->space_height;
			for (int i = 0; i <= (int) state->space_height; i += 1) {
				float y = y0 + dy * i;
				immediate_line({x0, y}, {x1, y}, grid_width, grid_color);
			}			
		}
		
		if (draw_positions) {
			constexpr float emitter_radius = 0.1;
			constexpr vec4 emitter_color = {0.1, 0.8, 0.8, 1};
			
			constexpr int attractor_sides = 30;
			constexpr vec4 attractor_color = {0.8, 0.1, 0.1, 1};
			
			constexpr float attractor_arrows_per_unit_length = 1;
			constexpr float attractor_arrows_size = 0.1;
			constexpr vec4 attractor_arrows_color = {0.6, 0.0, 0.1, 1};
			
			float t = fmod(glfwGetTime() * 1.0f, 1.0f);
			
			for (int a = 0; a < state->physics.attractor_count; a += 1) {
				auto attractor = &state->physics.attractors[a];
				if (attractor->active) {
					
					int arrow_count = attractor_arrows_per_unit_length * (2 * PI * attractor->radius);
					for (int i = 0; i < arrow_count; i += 1) {
						float angle = TAU * (i / (float) arrow_count);
						
						vec2 outward_direction = polar(1, angle);
						vec2 arrow_direction = sign(attractor->factor) * outward_direction;
						
						float movement_t = 1 - pow(1 - t, 5);
						float r0 = 0.1;
						float r1 = attractor->radius;
						float r = attractor->factor > 0 ? lerp(r1, r0, movement_t) : lerp(r0, r1, movement_t);
						
						vec4 arrows_color = attractor_arrows_color;
						arrows_color.w *= smoothstep2(1 - t);
						immediate_arrow_head(attractor->position + -outward_direction * r, arrow_direction, attractor_arrows_size, arrows_color);
					}
					
					world_drag_point(state, &attractor->position, 0.1, attractor_color);
					
					immediate_regular_polygon(attractor->position, attractor->radius, attractor_sides, vec4(attractor_color.xyz, 0.1));
				}
			}
			
			for (int e = 0; e < state->emitter_count; e += 1) {
				
				auto emitter = &state->emitters[e];
				if (emitter->active) {
					world_drag_point(state, &emitter->position, emitter_radius, emitter_color);
				}
			}
		}
		
		immediate_flush(&debug_render_state);
	}
}