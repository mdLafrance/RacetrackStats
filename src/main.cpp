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

#include <math.h>

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
#include <UI.h>

_GuiState GuiState;

_WorldState WorldState = { 
	WINDOW_DEFAULT_X,    // Starting window width
	WINDOW_DEFAULT_Y,    // Starting window height
	{0.0f, 0.0f, 0.0f},  // vec3 ambient color for the scene
	nullptr,		     // (string) path to root of executable (set in main)
	nullptr				 // (string) path to root of the mesh and texture data for mosport
};

static CSV* currentData;
static bool frameSizeChanged = false;

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
	// renderer->loadScene(std::string(WorldState.projectRoot) + "/resources/scenes/mosport_low.scene");

	// Initialize imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	io.FontGlobalScale = 1.3;
	// done imgui

	// Initialize some default GUI values
	::GuiState.fontSize = io.FontGlobalScale;
	::GuiState.fps = 0;

	// Load track map texture
	Texture* mapTexture = new Texture(std::string(WorldState.trackDataRoot) + "/ui/outline.png");

	::GuiState.mapTexture = mapTexture->getID();
	mapTexture->getWidthHeight(::GuiState.mapTextureDimensions[0], ::GuiState.mapTextureDimensions[1]);

	std::chrono::time_point<std::chrono::steady_clock> t1, t2;

	double dTime = 0;
	double seconds0 = glfwGetTime();

	glViewport(0, WorldState.windowY / 2, WorldState.windowX, WorldState.windowY);

	// Need to aggregate frame increase for playing over multiple frames, since can only tick in int amounts
	float tickTotal = 0;

	// If user hasn't ticked forward the playback, dont need to redraw
	int previousTick = -1;

	while (!glfwWindowShouldClose(window)) {
		if ((glfwGetTime() - seconds0) > 0.1) {
			GuiState.fps = 1/dTime;
			seconds0 = glfwGetTime();
		}

		// Process user input from the previous gui frame

		// If load new scene clicked
		if (GuiState.selected_menu_File_Open) {
			if (GuiState.dataFieldsEnabled != nullptr) {
				delete[] GuiState.dataFieldsEnabled;
			}

			currentData = new CSV(std::string(WorldState.projectRoot) + "/resources/laps/mosport1.csv");

			GuiState.numberOfDataTypes = currentData->getNumberOfDataTypes();
			GuiState.numberOfTimePoints = currentData->getNumberOfTimePoints();
			GuiState.dataFields = currentData->getOrderedData();
			GuiState.dataFieldsEnabled = new bool[GuiState.numberOfDataTypes];
			GuiState.sceneOpen = true;

			// FOR TESTING
			GuiState.TEST = new float[50];
			currentData->getBatchDataAsFloat("Distance", 0, 50, GuiState.TEST);

			// Start off with all data disabled
			memset(GuiState.dataFieldsEnabled, 0, GuiState.numberOfDataTypes);
		}

		// If timeline is set to 'play', add to the tick total (described above) and increment the timeline position
		if (GuiState.isPlaying) {
			tickTotal += abs(GuiState.playbackSpeed) * dTime; // sign agnostic to allow for backwards playing

			while (tickTotal > 1){
				GuiState.timelinePosition += 1;// GuiState.playbackSpeed;
				--tickTotal;
				// just for testing
				for (int i = 0; i < GuiState.numberOfDataTypes; i++){
					if (*(GuiState.dataFieldsEnabled + i)) {
						std::cout << '\r' << 
							GuiState.dataFields[i] << " at " << 
							GuiState.timelinePosition << " : " << 
							currentData->getData(i, GuiState.timelinePosition);
					}
				} // todo:: delete
			}
		} else {
			tickTotal = 0;
		}

		io.FontGlobalScale = GuiState.fontSize;


		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// If frame size changed, update appropriate fields
		// TODO: Update camera view, right now it just stretches
		if (frameSizeChanged) {
			glfwGetFramebufferSize(window, &::WorldState.windowX, &::WorldState.windowY);
			
			// For now, instead of framebuffer -> image, just render where the image would be
			// might not be a bad idea instead anyways
			glViewport(0, WorldState.windowY / 2, WorldState.windowX, WorldState.windowY);
		}

		t1 = std::chrono::steady_clock::now();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		
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