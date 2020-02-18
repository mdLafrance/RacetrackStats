#define STB_IMAGE_IMPLEMENTATION

#define WINDOW_TITLE "Racetrack Stats"

#ifdef MAKE_DLL
	#define DLL_EXPORT __declspec(dllexport)
#else 
	#define DLL_EXPORT // No effect from macro
#endif // DLL_EXPORT

#include <cstdlib>

#include <vector>
#include <fstream>
#include <chrono>
#include <iostream>
#include <chrono>
#include <thread>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <Renderer.h>
#include <Material.h>
#include <Utils.h>
#include <WorldState.h>
#include <Light.h>
#include <CSV.h>

_WorldState WorldState = { 
	WINDOW_DEFAULT_X,    // Starting window width
	WINDOW_DEFAULT_Y,    // Starting window height
	{0.0f, 0.0f, 0.0f},  // vec3 ambient color for the scene
	nullptr			     // (string) path to root of executable (set in main)
};

struct _GuiState {
	// Menu bar state
	bool selected_menu_File = false;
	bool selected_menu_File_Open = false;
};

const int menuBarHeight = 20;

void drawUI(_GuiState& state) {
	// Main menu drop down bar
	if (ImGui::BeginMainMenuBar()) {

		if (ImGui::BeginMenu("File", &state.selected_menu_File)) {
			ImGui::MenuItem("Open", NULL, &state.selected_menu_File_Open);
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	} // end main menu drop down bar

	// Main widget
	ImGui::SetNextWindowSize(ImVec2(WorldState.windowX, WorldState.windowY - menuBarHeight), 0);
	ImGui::SetNextWindowPos(ImVec2(0, menuBarHeight), 0, ImVec2(0,0));
	ImGui::Begin("MainWidget", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

	ImGui::Button("asdf", ImVec2(200,200));

	ImGui::End();
}

int main(int argc, char** argv) {
	// Init Context
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(WINDOW_DEFAULT_X, WINDOW_DEFAULT_Y, WINDOW_TITLE, NULL, NULL);

	if (window == NULL) {
		std::cout << "Failed to create window...\n";
		glfwTerminate();
		return -1;
	}

	auto windowResizeCallbackFunction = [](GLFWwindow* window, int x, int y) {
		// printf("Window resized to %d by %d\n", x, y); 
		::WorldState.windowX = x;
		::WorldState.windowY = y;
	};

	glfwSetWindowSizeCallback(window, windowResizeCallbackFunction);
	glfwSetErrorCallback(Utils::glfwErrorCallbackFunction);

	// Initialize renderer
	bool onMSI = std::getenv("MSI") != nullptr;

	// For personal convenience when working, this is defined on one of my machines
	if (onMSI) {
		::WorldState.projectRoot = "D:/Hacking/RacetrackStats";
	}
	else {
		::WorldState.projectRoot = "C:/Users/maxto/OneDrive/Documents/Hacking/RacetrackStats";
	}

	Renderer* renderer = new Renderer(window);

	if (onMSI) {
		renderer->loadScene(std::string(::WorldState.projectRoot) + "/resources/scenes/mosportTest.scene");
	}
	else { 
		renderer->loadScene(std::string(::WorldState.projectRoot) + "/resources/scenes/testingScene_laptop.scene");
	}
	// done renderer

	// Initialize imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// done imgui

	std::chrono::time_point<std::chrono::steady_clock> t1, t2;

	double dTime = 0;

	_GuiState GuiState;
	
	while (!glfwWindowShouldClose(window)) {
		t1 = std::chrono::steady_clock::now();

		glfwGetFramebufferSize(window, &::WorldState.windowX, &::WorldState.windowY);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		
		//renderer->tick(dTime);

		drawUI(GuiState);

		// Render dear imgui into screen
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();

		// Current frame finished

		t2 = std::chrono::steady_clock::now();
		dTime = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1000000.0f; // Microsecond conversion into fraction of second
	}

	// Cleanup

	delete renderer;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	return 0;
}