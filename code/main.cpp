
//
// This file just sets up window and ui initialization stuff.
//

#include "common.h"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

GLFWwindow* global_window;

void initialize();
void do_frame();

int main() {	
	// Create GLFW window.
	if (!glfwInit()) return 1;
	global_window = glfwCreateWindow(720, 480, "Sparkles!", nullptr, nullptr);
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
		
		ImGui::StyleColorsDark();
		
		ImGui_ImplGlfw_InitForOpenGL(global_window, true);
		ImGui_ImplOpenGL3_Init("#version 130");
	}
	
	initialize(); // Actual initialization function
	
	while (!glfwWindowShouldClose(global_window)) {
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
			glClearColor(0, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT);
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}
		
		glfwSwapBuffers(global_window);
		glfwPollEvents();
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