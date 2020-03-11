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
#include <WorldState.h>
#include <Light.h>
#include <CSV.h>
#include <Utils.h>

#include <UI.hpp>

// State of the GUI
_GuiState GuiState;

// State of the scene, used by the renderer
_WorldState WorldState = { 
	WINDOW_DEFAULT_X,    // Starting window width
	WINDOW_DEFAULT_Y,    // Starting window height
	{0.0f, 0.0f, 0.0f},  // vec3 ambient color for the scene
	nullptr,		     // (string) path to root of executable (set in main)
	nullptr				 // (string) path to root of the mesh and texture data for mosport
};

// Internal state variables
static CSV* currentData;
static bool frameSizeChanged = false;
static bool doStopLoadingThread = false;

// Handler for shortcuts used to navigate ui
void keyPressCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// Spacebar to play/pause
	if (key ==  GLFW_KEY_SPACE && action == GLFW_PRESS) {
		GuiState.isPlaying = !GuiState.isPlaying;
	}

	// Arrow keys to nudge timeline
	if (key ==  GLFW_KEY_LEFT && action == GLFW_PRESS) GuiState.timelinePosition -= GuiState.tickSkipAmount;
	if (key ==  GLFW_KEY_RIGHT && action == GLFW_PRESS) GuiState.timelinePosition += GuiState.tickSkipAmount;
}

// void runLoadingBar(const std::string& name, const float* progress){
// 	GLFWwindow* window = glfwCreateWindow(400, 500, "TEST WINDOW", NULL, NULL);

// 	IMGUI_CHECKVERSION();
// 	ImGui::CreateContext();
// 	ImGuiIO& io = ImGui::GetIO(); (void)io;
// 	ImGui::StyleColorsDark();
// 	ImGui_ImplGlfw_InitForOpenGL(window, true);
// 	ImGui_ImplOpenGL3_Init("#version 330");

// 	while (*progress <= 0.99f && !doStopLoadingThread) {
// 		ImGui_ImplOpenGL3_NewFrame();
// 		ImGui_ImplGlfw_NewFrame();
// 		ImGui::NewFrame();

// 		ImGui::Begin(("Loading " + name).c_str());
// 		ImGui::Text(std::to_string(*progress).c_str());
// 		ImGui::End();

// 		// Render dear imgui onto screen
// 		ImGui::Render();
// 		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

// 		glfwPollEvents();
// 		glfwSwapBuffers(window);
// 	}

// 	glfwDestroyWindow(window);
// }

int main(int argc, char** argv) {
	std::string executableDirectory = Utils::getFileInfo(*(argv)).directory;
	std::cout << "Launching racetracks stats... (" << executableDirectory << ")" << std::endl << std::endl;

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

	glfwMakeContextCurrent(window);

	// Initialize glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD\n";
	}

	// Initial opengl settings
	glFrontFace(GL_CCW);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Resize window just sets a flag, which is checked in loop
	auto windowResizeCallbackFunction = [](GLFWwindow* window, int x, int y) {
		frameSizeChanged = true;
	};

	// Set glfw callbacks
	glfwSetKeyCallback(window, keyPressCallback);
	glfwSetWindowSizeCallback(window, windowResizeCallbackFunction);
	glfwSetErrorCallback(Utils::glfwErrorCallbackFunction);

	bool onMSI = std::getenv("MSI") != nullptr;

	// For personal convenience when working, this is defined on one of my machines
	// TODO: In final, this should be some relative path to the execuatable
	if (onMSI) {
		::WorldState.projectRoot = "D:/Hacking/RacetrackStats";
		::WorldState.trackDataRoot = "D:/Hacking/RacetrackStats";
	}
	else {
		::WorldState.projectRoot = "C:/Users/maxto/OneDrive/Documents/Hacking/RacetrackStats";
		::WorldState.trackDataRoot = "C:/Users/maxto/OneDrive/Documents/mosport";
	}

	// Initialize renderer
	Renderer* renderer = new Renderer(window);

	// Initialize imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// Set up some initial GUI data
	io.FontGlobalScale = 1.3;
	::GuiState.fontSize = io.FontGlobalScale;
	setGuiOptionsToDefault(GuiState);
	glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, ::GuiState.glLineWidthRange);

	// Load track map texture to GUI
	// Texture* mapTexture = new Texture(std::string(WorldState.projectRoot) + "/resources/ui/map.png");
	// ::GuiState.mapTexture = mapTexture->getID();
	// mapTexture->getWidthHeight(::GuiState.mapTextureDimensions[0], ::GuiState.mapTextureDimensions[1]);

	// Setup renderer to only draw in top half of screen
	glViewport(0, WorldState.windowY / 2, WorldState.windowX, WorldState.windowY);

	// Draw one frame of GUI to look clean while loading
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	drawUI(GuiState);
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glfwSwapBuffers(window);

	// Load Mosport Scene
	doStopLoadingThread = false;
	// std::thread loadingBarThread(runLoadingBar, "SCENE!", &renderer->progress);
	renderer->loadScene(std::string(WorldState.projectRoot) + "/resources/scenes/mosport_low.scene");
	doStopLoadingThread = true;

	//
	// Initialize variables that will fluctuate over the runtime of the scene
	//

	// To accomodate for brightness slider, kind of just for fun
	float originalAmbientLight[3];
	memcpy(originalAmbientLight, WorldState.ambientLight, 3 * sizeof(float));

	// Used to time deltaTime since last frame
	std::chrono::time_point<std::chrono::steady_clock> t1, t2;

	double dTime = 0;
	double seconds0 = glfwGetTime();

	// Used to calculate timeline playing
	float tickTotal = 0;

	//
	// Main loop
	// 

	while (!glfwWindowShouldClose(window)) {
		t1 = std::chrono::steady_clock::now();

		// Calculate fps of last frame, pass to gui
		if ((glfwGetTime() - seconds0) > 0.1) { // Dont need to do this every frame, instead only every .1 seconds
			GuiState.fps = 1/dTime;
			seconds0 = glfwGetTime();
		}

		//
		// Process user input to the gui from the previous frame
		//

		// If load new scene clicked
		if (GuiState.selected_menu_File_Open) {
			// Reset GUI values to track data fields
			if (GuiState.dataFieldsEnabled != nullptr) delete[] GuiState.dataFieldsEnabled;
			if (currentData != nullptr) delete currentData;

			// Load CSV data
			currentData = new CSV(std::string(WorldState.projectRoot) + "/resources/laps/mosport1.csv");

			// Update GUI to match the parameters of the new CSV file
			GuiState.numberOfDataTypes = currentData->getNumberOfDataTypes();
			GuiState.numberOfTimePoints = currentData->getNumberOfTimePoints();
			GuiState.dataFields = currentData->getOrderedData();
			GuiState.dataFieldsEnabled = new bool[GuiState.numberOfDataTypes];
			GuiState.sceneOpen = true;

			// Start off with all data disabled
			memset(GuiState.dataFieldsEnabled, 0, GuiState.numberOfDataTypes);
		}

		// If timeline is set to 'play', add to the tick total and increment the timeline position
		if (GuiState.isPlaying) {
			tickTotal += abs(GuiState.playbackSpeed) * dTime; // abs value to allow for backwards playing

			while (tickTotal > 1){
				GuiState.timelinePosition += Utils::sign(GuiState.playbackSpeed);
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

		// Update line width if changed
		if (GuiState.lineWidthChanged) {
			renderer->setLineWidth(GuiState.lineWidth);
			GuiState.lineWidthChanged = false;
		}

		//
		// Handle resize of window, and update to camera settings through ui, both of need a new camera view matrix
		//

		// If user dragged window size around
		if (frameSizeChanged) {
			glfwGetFramebufferSize(window, &::WorldState.windowX, &::WorldState.windowY);
			glViewport(0, WorldState.windowY / 2, WorldState.windowX, WorldState.windowY);

			GuiState.cameraSettingsChanged = true; // force new perpective matrix
			frameSizeChanged = false;
		}

		// If camera settings changed, build new view matrix to reflect these changes
		if (GuiState.cameraSettingsChanged) {
			renderer->getMainCamera()->setPerspectiveProjMatrix(45.0f + GuiState.FOV, (float)WorldState.windowX/WorldState.windowY, GuiState.nearClipPlane, GuiState.farClipPlane);
			GuiState.cameraSettingsChanged = false;
		}

		// Adjust brightness based on brightness slider
		WorldState.ambientLight[0] = originalAmbientLight[0] + GuiState.brightness;
		WorldState.ambientLight[1] = originalAmbientLight[1] + GuiState.brightness;
		WorldState.ambientLight[2] = originalAmbientLight[2] + GuiState.brightness;

		// Update ui font size
		io.FontGlobalScale = GuiState.fontSize;

		// Draw renderable elements

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (!renderer->loading){
			renderer->tick(dTime);
		}

		// Draw GUI elements
 		ImGui_ImplOpenGL3_NewFrame();
 		ImGui_ImplGlfw_NewFrame();
 		ImGui::NewFrame();

		 drawUI(GuiState);

 		ImGui::Render();
 		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Current frame finished

		glfwSwapBuffers(window);
		glfwPollEvents();

		t2 = std::chrono::steady_clock::now();
		dTime = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1000000.0f; // Microsecond conversion into fraction of second
	}

	// Cleanup

	delete renderer; // Deletes internal scene data

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);

	return 0;
}