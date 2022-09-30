//
// This file just sets up window and ui initialization stuff.
//


#if _WIN32 
#include <windows.h>
#else
#error "This library cannot be compiled for the target platform."
#endif

#include "backends/imgui_impl_glfw.h"

#if GRAPHICS_OPENGL
#include "glad/gl.h"
#include "backends/imgui_impl_opengl3.h"
#define GRAPHICS_IMGUI_INIT() { ImGui_ImplGlfw_InitForOpenGL(global_window, true); ImGui_ImplOpenGL3_Init("#version 130"); }
#define GRAPHICS_IMGUI_SHUTDOWN() ImGui_ImplOpenGL3_Shutdown();
#define GRAPHICS_IMGUI_NEW_FRAME() ImGui_ImplOpenGL3_NewFrame();
#define GRAPHICS_IMGUI_RENDER() ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#else 
#error "No graphics backend was defined."
#endif

#include "common.h"

void system_sleep_ms(int milliseconds);

GLFWwindow* global_window;
TimeInfo global_time;
extern unsigned int default_font_data_size;
extern unsigned int default_font_data[115744/4];

bool initialize();
void do_frame();

int main() {	
	if (!glfwInit()) return 1;
	
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* vidmode = glfwGetVideoMode(monitor);
	
	int window_width = (int) (vidmode->width);
	int window_height = (int) (vidmode->height);
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_MAXIMIZED, true);
	
	global_window = glfwCreateWindow(window_width, window_height, "Sparkles!", nullptr, nullptr);
	
	if (false) {
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
		
		GRAPHICS_IMGUI_INIT();
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
			GRAPHICS_IMGUI_NEW_FRAME();
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
			
			GRAPHICS_IMGUI_RENDER();
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
	GRAPHICS_IMGUI_SHUTDOWN();
	
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	
	// Shutdown GLFW
	glfwDestroyWindow(global_window);
	glfwTerminate();
	
	return 0;
}

#if _WIN32
void system_sleep_ms(int milliseconds) {
	timeBeginPeriod(1);
	Sleep(milliseconds);
}
#endif

