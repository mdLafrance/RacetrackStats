#define STB_IMAGE_IMPLEMENTATION

#define WINDOW_TITLE "Racetrack Stats"

// #ifdef MAKE_DLL
// 	#define DLL_EXPORT __declspec(dllexport)
// #else 
// 	#define DLL_EXPORT // No effect from macro
// #endif // DLL_EXPORT

#define WINDOW_DEFAULT_X 1000
#define WINDOW_DEFAULT_Y 1000

#define RENDER_DEFAULT_X 1000
#define RENDER_DEFAULT_Y 1000

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
	nullptr,		     // (string) path to root of executable (set in main)
	nullptr				 // (string) path to root of the mesh and texture data for mosport
};

static bool frameSizeChanged = false;

struct _GuiState {
	// Global states
	GLuint viewportRenderTarget;
	GLuint mapTexture;
	float mapTextureDimensions[2] = { 0,0 };
	float viewportRenderTargetDimensions[2] = { 0,0 };
	float fontSize;
	int padding = 17;
	double fps; // Set in loop

	// Menu bar state
	bool selected_menu_File = false;
	bool selected_menu_File_Open = false;
};

_GuiState GuiState;

ImVec2 addImVec2(const ImVec2& a, const ImVec2& b) { // + operator not defined??
	return ImVec2(a[0] + b[0], a[1] + b[1]);
}

void drawUI(_GuiState& state) {
	// Main menu drop down bar
	int menuBarHeight = state.fontSize * 10 + 10;

	float X, Y;
	X = WorldState.windowX;
	Y = WorldState.windowY;

	if (ImGui::BeginMainMenuBar()) {

		if (ImGui::BeginMenu("File", &state.selected_menu_File)) {
			ImGui::MenuItem("Open", NULL, &state.selected_menu_File_Open);
			if (state.selected_menu_File_Open) {
				std::cout << "Open file!" << std::endl;
				state.selected_menu_File_Open = false;
			}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	} // Main menu drop down bar

	/*

		Until can fix inverted render texture problem, just draw raw to screen

	// Viewport widget
	ImGui::SetNextWindowSize(ImVec2(WorldState.windowX, WorldState.windowY/2 - menuBarHeight), 0);
	ImGui::SetNextWindowPos(ImVec2(0, menuBarHeight), 0, ImVec2(0,0));
	ImGui::Begin("Viewport", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

	// Display render texture
	ImGui::Image((void*)renderTexture, ImVec2(RENDER_DEFAULT_X, RENDER_DEFAULT_Y));

	ImGui::End(); // Viewport Widget
	*/

	//ImGui::Text("%f fps", state.fps);

	// Data panel
	ImGui::SetNextWindowSize(ImVec2(X, Y / 2));
	ImGui::SetNextWindowPos(ImVec2(0, Y/2), 0, ImVec2(0, 0));
	ImGui::Begin("Data", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::End(); // Data Panel

	// Overlayed map image
	ImVec2 mapDimensions(state.mapTextureDimensions[0], state.mapTextureDimensions[1]); // Add vertical padding for title
	ImGui::SetNextWindowSize(addImVec2(mapDimensions, ImVec2(0, menuBarHeight + state.padding)), 0);
	ImGui::SetNextWindowPos(ImVec2(X - mapDimensions[0], menuBarHeight));
	ImGui::Begin("Map", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
	ImGui::Image((void*)state.mapTexture, mapDimensions);
	ImGui::End(); // Map
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
		frameSizeChanged = true;
	};

	glfwSetWindowSizeCallback(window, windowResizeCallbackFunction);
	glfwSetErrorCallback(Utils::glfwErrorCallbackFunction);

	// Initialize renderer
	bool onMSI = std::getenv("MSI") != nullptr;

	// For personal convenience when working, this is defined on one of my machines
	if (onMSI) {
		::WorldState.projectRoot = "D:/Hacking/RacetrackStats";
		::WorldState.trackDataRoot = "D:/Hacking/RacetrackStats";
	}
	else {
		::WorldState.projectRoot = "C:/Users/maxto/OneDrive/Documents/Hacking/RacetrackStats";
		::WorldState.trackDataRoot = "C:/Users/maxto/OneDrive/Documents/mosport";
	}

	Renderer* renderer = new Renderer(window);

	/*
	if (onMSI) {
		renderer->loadScene(std::string(::WorldState.projectRoot) + "/resources/scenes/mosportTest.scene");
	}
	else {
		renderer->loadScene(std::string(::WorldState.projectRoot) + "/resources/scenes/testingScene_laptop.scene");
	}
	*/

	// Initialize imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	io.FontGlobalScale = 1.8;
	// done imgui

	// Load track map texture
	Texture* mapTexture = new Texture(std::string(WorldState.trackDataRoot) + "/ui/outline.png");

	::GuiState.fontSize = io.FontGlobalScale;
	::GuiState.fps = 0;

	::GuiState.viewportRenderTargetDimensions[0] = RENDER_DEFAULT_X;
	::GuiState.viewportRenderTargetDimensions[1] = RENDER_DEFAULT_Y;

	::GuiState.mapTexture = mapTexture->getID();
	mapTexture->getWidthHeight(::GuiState.mapTextureDimensions[0], ::GuiState.mapTextureDimensions[1]);

	std::chrono::time_point<std::chrono::steady_clock> t1, t2;

	double dTime = 0;
	double seconds0 = glfwGetTime();

	// TODO: Move this into small class

	/*
	GLuint FBO, zbuffer;

	// Generate frame buffer
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	// Generate empty render texture
	glGenTextures(1, &renderTexture);
	glBindTexture(GL_TEXTURE_2D, renderTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, RENDER_DEFAULT_X, RENDER_DEFAULT_Y, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Attach to framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture, 0);

	// Generate and attach zbuffer
	glGenRenderbuffers(1, &zbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, zbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_ATTACHMENT, RENDER_DEFAULT_X, RENDER_DEFAULT_Y);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// done!
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	*/

	glViewport(0, WorldState.windowY / 2, WorldState.windowX, WorldState.windowY);

	while (!glfwWindowShouldClose(window)) {
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (frameSizeChanged) {
			glfwGetFramebufferSize(window, &::WorldState.windowX, &::WorldState.windowY);

			glViewport(0, WorldState.windowY / 2, WorldState.windowX, WorldState.windowY);
			// Regenerate FBO and other resize events
		}

		t1 = std::chrono::steady_clock::now();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if ((glfwGetTime() - seconds0) > 0.1) {
			GuiState.fps = 1/dTime;
			seconds0 = glfwGetTime();
		}
		
		renderer->tick(dTime);

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