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

void keyPressCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// Spacebar to play/pause
	if (key ==  GLFW_KEY_SPACE && action == GLFW_PRESS) {
		GuiState.isPlaying = !GuiState.isPlaying;
	}

	// Arrow keys to nudge timeline
	if (key ==  GLFW_KEY_LEFT && action == GLFW_PRESS) GuiState.timelinePosition -= GuiState.tickSkipAmount;
	if (key ==  GLFW_KEY_RIGHT && action == GLFW_PRESS) GuiState.timelinePosition += GuiState.tickSkipAmount;
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
	renderer->loadScene(std::string(WorldState.projectRoot) + "/resources/scenes/mosport_low.scene");
	// renderer->loadScene(std::string(WorldState.projectRoot) + "/resources/scenes/testingScene_laptop.scene");

	Transform* rootTransform = new Transform();

	Transform* tempTransform;

	// TODO: maybe overhaul object->mesh relationship, made that while learning
	// for (std::pair<std::string, Object*> p : renderer->objects) {
	// 	if (p.first.rfind("BMW_M3_E92.", 0) == 0) { // If startswith bmw prefix
	// 		tempTransform = p.second->transform;
	// 		tempTransform->setParent(rootTransform);
	// 	}
	// }

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

	setGuiOptionsToDefault(GuiState);

	glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, ::GuiState.glLineWidthRange);

	// Load track map texture
	Texture* mapTexture = new Texture(std::string(WorldState.trackDataRoot) + "/ui/outline.png");

	::GuiState.mapTexture = mapTexture->getID();
	mapTexture->getWidthHeight(::GuiState.mapTextureDimensions[0], ::GuiState.mapTextureDimensions[1]);

	glViewport(0, WorldState.windowY / 2, WorldState.windowX, WorldState.windowY);

	// Initialize variables that will fluctuate over the runtime of the scene

	// To accomodate for brightness slider, kind of just for fun
	float originalAmbientLight[3];
	memcpy(originalAmbientLight, WorldState.ambientLight, 3 * sizeof(float));

	std::chrono::time_point<std::chrono::steady_clock> t1, t2;

	double dTime = 0;
	double seconds0 = glfwGetTime();

	// Need to aggregate frame increase for playing over multiple frames, since can only tick in int amounts
	float tickTotal = 0;

	// If user hasn't ticked forward the playback, dont need to redraw
	int previousTick = -1;

	// Set glfw callbacks
	glfwSetKeyCallback(window, keyPressCallback);
	glfwSetWindowSizeCallback(window, windowResizeCallbackFunction);
	glfwSetErrorCallback(Utils::glfwErrorCallbackFunction);

	// renderer->getMainCamera()->transform->setTranslation(glm::vec3(577.466, -12.166, 6.49934));

	while (!glfwWindowShouldClose(window)) {

		t1 = std::chrono::steady_clock::now();

		// Calculate fps of last frame, pass to gui
		if ((glfwGetTime() - seconds0) > 0.1) {
			GuiState.fps = 1/dTime;
			seconds0 = glfwGetTime();
		}

		//
		// Process user input to the gui from the previous frame
		//

		// If load new scene clicked
		if (GuiState.selected_menu_File_Open) {
			if (GuiState.dataFieldsEnabled != nullptr) delete[] GuiState.dataFieldsEnabled;
			if (currentData != nullptr) delete currentData;

			currentData = new CSV(std::string(WorldState.projectRoot) + "/resources/laps/mosport1.csv");

			GuiState.numberOfDataTypes = currentData->getNumberOfDataTypes();
			GuiState.numberOfTimePoints = currentData->getNumberOfTimePoints();
			GuiState.dataFields = currentData->getOrderedData();
			GuiState.dataFieldsEnabled = new bool[GuiState.numberOfDataTypes];
			GuiState.sceneOpen = true;

			// Start off with all data disabled
			memset(GuiState.dataFieldsEnabled, 0, GuiState.numberOfDataTypes);
		}

		// If timeline is set to 'play', add to the tick total (described above) and increment the timeline position
		if (GuiState.isPlaying) {
			tickTotal += abs(GuiState.playbackSpeed) * dTime; // sign agnostic to allow for backwards playing

			while (tickTotal > 1){
				GuiState.timelinePosition += Utils::signInt(GuiState.playbackSpeed);
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

		if (GuiState.lineWidthChanged) {
			renderer->setLineWidth(GuiState.lineWidth);
			GuiState.lineWidthChanged = false;
		}

		//
		// Handle resize of window, and update to camera settings through ui
		//

		// If user dragged window size around
		if (frameSizeChanged) {
			glfwGetFramebufferSize(window, &::WorldState.windowX, &::WorldState.windowY);
			glViewport(0, WorldState.windowY / 2, WorldState.windowX, WorldState.windowY);

			GuiState.cameraSettingsChanged = true; // force new perpective matrix
			frameSizeChanged = false;
		}

		// If camera settings changed, build new view matrix
		if (GuiState.cameraSettingsChanged) {
			renderer->getMainCamera()->setPerspectiveProjMatrix(45.0f + GuiState.FOV, (float)WorldState.windowX/WorldState.windowY, GuiState.nearClipPlane, GuiState.farClipPlane);
			GuiState.cameraSettingsChanged = false;
		}

		// Adjust brightness based on brightness slider
		WorldState.ambientLight[0] = originalAmbientLight[0] + GuiState.brightness;

		renderer->getMainCamera()->transform->setParent(rootTransform);
		WorldState.ambientLight[1] = originalAmbientLight[1] + GuiState.brightness;
		WorldState.ambientLight[2] = originalAmbientLight[2] + GuiState.brightness;

		// Update ui font size
		io.FontGlobalScale = GuiState.fontSize;

		// done gui handling

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// std::cout << '\r' << vec3ToString(renderer->getMainCamera()->transform->position());

		// Draw renderable elements
		renderer->tick(dTime);
		renderer->drawLine(glm::vec3(-50, 50, 50), glm::vec3(50, 0, -50), glm::vec3(1.0f, 0.0f, 1.0f));

		// Minimized window (0 by 0) causes imgui to crash, currently this is a test for fix
		int windowSize[2];
		glfwGetFramebufferSize(window, &windowSize[0], &windowSize[1]);

		if (true){//!(windowSize[0] == 0 || windowSize[1] == 0)) {
			// std::cout << windowSize[0] << ' ' << windowSize[1] << std::endl;
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			// Draw GUI elements
			drawUI(GuiState);

			// Render dear imgui onto screen
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}

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