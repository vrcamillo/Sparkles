
//
// This file just sets up window and ui initialization stuff.
//

#include "common.h"

// #random_number_cleanup
#include <stdlib.h> // srand
#include <time.h> // time

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

GLFWwindow* global_window;
TimeInfo global_time;

bool initialize();
void do_frame();

int main() {
	
	// Initialize random number generator seed to the current time, so that every time we run the program we get different results.
	// Eventually we will want to be very careful about random numbers, but for the mean time, we just use c standard lib.
	// #random_number_cleanup
	srand(time(NULL));
	
	
	
	if (!glfwInit()) return 1;
	
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* vidmode = glfwGetVideoMode(monitor);
	
	int window_width = (int) (vidmode->width * 3) / 4;
	int window_height = (int) (vidmode->height * 3) / 4;
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	global_window = glfwCreateWindow(window_width, window_height, "Sparkles!", nullptr, nullptr);
	
	{
		// Centralize window.
		int x = (vidmode->width - window_width) / 2;
		int y = (vidmode->height - window_height) / 2;
		glfwSetWindowPos(global_window, x, y);
	}
	
	glfwMakeContextCurrent(global_window);
	glfwSwapInterval(1); // v-sync on
	
	// Initialize OpenGL functions.
	if (!gladLoadGL(glfwGetProcAddress)) {
		return 1;
	}
	
	{
		// Initialize ImGui
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		io.IniFilename = nullptr; // Remove annoying ini file.
		io.Fonts->AddFontFromMemoryCompressedTTF(default_font_data, default_font_data_size, 16.0f); // Remove ugly default imgui font.
		
		ImGui::StyleColorsDark();
		
		ImGui_ImplGlfw_InitForOpenGL(global_window, true);
		ImGui_ImplOpenGL3_Init("#version 130");
	}
	
	// Initialize time global variable.
	global_time.min_dt = 1.0f / vidmode->refreshRate; // Use the current monitor as reference.
	global_time.max_dt = 1.0f; // #hardcoded to 1 second.
	global_time.dt = 0.0f;
	
	bool initialized = initialize(); // Actual initialization function
	if (!initialized) return 1;
	
	while (!glfwWindowShouldClose(global_window)) {
		double frame_start_time = glfwGetTime();
		
		glfwPollEvents();
		
		{
			// ImGui pre frame stuff
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
		}
		
		do_frame(); // Actual frame function.
		
		{
			// ImGui post frame stuff
			ImGui::Render();
			int display_w, display_h;
			glfwGetFramebufferSize(global_window, &display_w, &display_h);
			glViewport(0, 0, display_w, display_h);
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}
		
		glfwSwapBuffers(global_window);
		
		double frame_end_time = glfwGetTime();
		float frame_dt = (float) (frame_end_time - frame_start_time);
		
		if (frame_dt < global_time.min_dt) {
			float seconds_to_sleep = global_time.min_dt - frame_dt;
			int ms_to_sleep = (int) (seconds_to_sleep * 1000);
			system_sleep_ms(ms_to_sleep);
			
			frame_dt = global_time.min_dt;
		} 
		
		if (frame_dt > global_time.max_dt) {
			frame_dt = global_time.max_dt;
		}
		
		global_time.dt = frame_dt;
	}
	
	// Shutdown ImGui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	
	// Shutdown GLFW
	glfwDestroyWindow(global_window);
	glfwTerminate();
	
	return 0;
}