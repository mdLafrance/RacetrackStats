#define STB_IMAGE_IMPLEMENTATION // Needed for stbi image loading

#define WINDOW_TITLE "Racetrack Stats"

// #ifdef MAKE_DLL
// 	#define DLL_EXPORT __declspec(dllexport)
// #else 
// 	#define DLL_EXPORT // No effect from macro
// #endif // DLL_EXPORT

#define WINDOW_DEFAULT_X 1000
#define WINDOW_DEFAULT_Y 850

#include <math.h>

#include <assert.h>

// TODO: Uncomment this for release
// #define NDEBUG 

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

#include <portable-file-dialogs.h> // 3rd party header library that creates system dialogs

#include <Renderer.h>
#include <Material.h>
#include <WorldState.h>
#include <Light.h>
#include <CSV.h>
#include <Utils.h>

#include <UI.hpp>

// State of the GUI
_GuiState GuiState;

// Cameras available to switch between
static const std::vector<std::string> cameras = { "Follow", "Car", "Overhead" };

// State of the scene, used by the renderer
_WorldState WorldState = { 
	WINDOW_DEFAULT_X,    // Starting window width
	WINDOW_DEFAULT_Y,    // Starting window height
	WINDOW_DEFAULT_X,    // Proper values for renderer w h get filled in main
	WINDOW_DEFAULT_Y,     
	{0.0f, 0.0f, 0.0f},  // vec3 ambient color for the scene
	nullptr,		     // (string) path to root of executable (set in main)
	nullptr				 // (string) path to root of the mesh and texture data for mosport
};

// Internal state variables
const static char* exeLocation;
const static char* trackDataLocation;

static Renderer* renderer;
static CSV* currentData;
ImGuiIO* imguiIO;

static bool frameSizeChanged = false;

bool mouseMoved;
double dMouseX = 0;
double dMouseY = 0;

// Handler for shortcuts used to navigate the ui
void keyPressCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// Escape to close the program
	// TODO: remove in final?
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

	// Spacebar to play/pause
	if (key ==  GLFW_KEY_SPACE && action == GLFW_PRESS) GuiState.isPlaying = !GuiState.isPlaying;

	// Arrow keys to nudge timeline
	if (key ==  GLFW_KEY_LEFT && action == GLFW_PRESS) GuiState.timelinePosition = Utils::clamp(GuiState.timelinePosition - GuiState.tickSkipAmount, 0, GuiState.numberOfTimePoints);
	if (key ==  GLFW_KEY_RIGHT && action == GLFW_PRESS) GuiState.timelinePosition = Utils::clamp(GuiState.timelinePosition + GuiState.tickSkipAmount, 0, GuiState.numberOfTimePoints);

	// Number keys to switch between cameras
	if (key == GLFW_KEY_1 && action == GLFW_PRESS) renderer->setMainCamera("Follow");
	if (key == GLFW_KEY_2 && action == GLFW_PRESS) renderer->setMainCamera("Car");
	if (key == GLFW_KEY_3 && action == GLFW_PRESS) renderer->setMainCamera("Overhead");

	// Reset orientation of main cam
	if (key == GLFW_KEY_R && action == GLFW_PRESS) renderer->getMainCamera()->transform->setRotation(0, glm::vec3(0,1,0));
}

void mouseMoveCallback(GLFWwindow* window, double xPos, double yPos) {
	static double lastMouseX;
	static double lastMouseY;

	dMouseX = xPos - lastMouseX;
	dMouseY = yPos - lastMouseY;

	lastMouseX = xPos;
	lastMouseY = yPos;

	mouseMoved = true;
}

void cleanup() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

int main(int argc, char** argv) {
	std::string executableDirectory = Utils::getFileInfo(*argv).directory;
	std::cout << "Launching racetracks stats... (" << executableDirectory << ")" << std::endl << std::endl;

	 //
	// Initialize glfw and opengl
	//

	// Init Context
	glfwInit();

	// NOTE: This crashes Multithread ImGui for some reason
	// glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	// glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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
	glfwSetCursorPosCallback(window, mouseMoveCallback);

	bool onMSI = std::getenv("MSI") != nullptr;

	// For personal convenience when working, this is defined on one of my machines
	// TODO: In final, this should be some relative path to the execuatable
	if (onMSI) {
		::WorldState.projectRoot = "D:/Hacking/RacetrackStats";
		::WorldState.trackDataRoot = "D:/Hacking/RacetrackStats/Mosport";
	}
	else {
		::WorldState.projectRoot = "C:/Users/maxto/OneDrive/Documents/Hacking/RacetrackStats";
		::WorldState.trackDataRoot = "C:/Users/maxto/OneDrive/Documents/Hacking/RacetrackStats/Mosport";
	}

	// If on a different machine, set these parameters here
	// ::WorldState.projectRoot = 
	// ::WorldState.trackDataRoot = 

	//
	// Initialize imgui
	//

	IMGUI_CHECKVERSION();
	ImGuiContext* imguiContext = ImGui::CreateContext();
	ImGui::SetCurrentContext(imguiContext);
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// Set up some initial GUI data
	// ::GuiState.io = &ImGui::GetIO();
	imguiIO = &ImGui::GetIO();
	imguiIO->FontGlobalScale = 1.3;

	// ::GuiState.io->FontGlobalScale = 1.3;
	setGuiOptionsToDefault(GuiState);
	glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, ::GuiState.glLineWidthRange);

	// Load track map texture to GUI
	::GuiState.mapTexture = new Texture(std::string(WorldState.projectRoot) + "/resources/ui/map.png");

	// Load the texures used for the timeline control buttons
	*(::GuiState.playButtonTextures + 0) = new Texture(std::string(WorldState.projectRoot) + "/resources/ui/play_button.png");
	*(::GuiState.playButtonTextures + 1) = new Texture(std::string(WorldState.projectRoot) + "/resources/ui/play_button_hovered.png");
	*(::GuiState.playButtonTextures + 2) = new Texture(std::string(WorldState.projectRoot) + "/resources/ui/play_button_pressed.png");

	*(::GuiState.pauseButtonTextures + 0) = new Texture(std::string(WorldState.projectRoot) + "/resources/ui/pause_button.png");
	*(::GuiState.pauseButtonTextures + 1) = new Texture(std::string(WorldState.projectRoot) + "/resources/ui/pause_button_hovered.png");
	*(::GuiState.pauseButtonTextures + 2) = new Texture(std::string(WorldState.projectRoot) + "/resources/ui/pause_button_pressed.png");

	*(::GuiState.skipButtonTextures + 0) = new Texture(std::string(WorldState.projectRoot) + "/resources/ui/skip_button.png");
	*(::GuiState.skipButtonTextures + 1) = new Texture(std::string(WorldState.projectRoot) + "/resources/ui/skip_button_hovered.png");
	*(::GuiState.skipButtonTextures + 2) = new Texture(std::string(WorldState.projectRoot) + "/resources/ui/skip_button_pressed.png");

	// Run window to let user select the scene they wish to load
	bool userCancelledProgram = false;
	std::string selectedScene = runSceneSelectWindow(window, std::string(WorldState.projectRoot) + "/resources/scenes", userCancelledProgram);

	// If user closed the select window or clicked the cancel button, go right to clean up stage
	if (userCancelledProgram) {
		cleanup();
		return 0;
	}

	std::cout << "User selected " << selectedScene << std::endl;

	glfwSetWindowPos(window, 40, 40); // Not necessary, just makes the window appear in a consistent spot

	WorldState.rendererX = WorldState.windowX;
	WorldState.rendererY = WorldState.windowY / 2;

	// Setup renderer to only draw in top half of screen
	glViewport(0, WorldState.windowY - WorldState.rendererY, WorldState.windowX, WorldState.rendererY);

	// Initialize renderer
	renderer = new Renderer(window);

	// Draw one frame of GUI to look clean while loading
	drawUI(GuiState);

	glfwSwapBuffers(window);

	// Load Mosport Scene
	renderer->loadScene(std::string(WorldState.projectRoot) + "/resources/scenes/" + selectedScene);

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

	// Set up default cameras
	Camera* followCam = new Camera(Perspective);
	Camera* carCam = new Camera(Perspective);
	Camera* overHeadCam = new Camera(Orthographic);

	followCam->transform->setTranslation(glm::vec3(-50, 0, 0));
	carCam->transform->setTranslation(glm::vec3(50, 50, -20));
	overHeadCam->transform->setTranslation(glm::vec3(0, 0, -100));

	renderer->registerCamera("Follow", followCam);
	renderer->registerCamera("Car", carCam);
	renderer->registerCamera("Overhead", overHeadCam);

	renderer->setMainCamera("Follow");

	//
	// Main loop
	// 

	while (!glfwWindowShouldClose(window)) {
		t1 = std::chrono::steady_clock::now();

		// Calculate fps of last frame, pass to gui
		float glfwTime = glfwGetTime();
		if ((glfwTime - seconds0) > 0.1) { // Dont need to do this every frame, instead only every .1 seconds
			GuiState.fps = 1/dTime;
			seconds0 = glfwTime;
		}

		//
		// Process user input to the gui from the previous frame
		//

		if (mouseMoved) {
			if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) {
				std::cout << dMouseX << " " << dMouseY << std::endl;

				Transform* mainCamTransform = renderer->getMainCamera()->transform;

				// Mouse sensitivity is multiplied by 100 in gui
				mainCamTransform->rotate(dTime * 0.01f * GuiState.mouseSensitivity * -dMouseX, glm::vec3(0,1,0));
				mainCamTransform->rotate(dTime * 0.01f * GuiState.mouseSensitivity * dMouseY, mainCamTransform->right());
			}

			mouseMoved = false;
		}

		// If load new scene clicked
		if (GuiState.selected_menu_File_Open) {
			// Launch open file dialog
			pfd::open_file fd = pfd::open_file::open_file("Open CSV File", "", {"All Files", "*.csv" }, false);
			std::vector<std::string> choice = fd.result();

			// Returned list is empty if user cancelled window, else it contains what the user selected

			if (choice.size() != 0) { 
			// Reset GUI values to track data fields
			if (GuiState.dataFieldsEnabled != nullptr) delete[] GuiState.dataFieldsEnabled;
			if (currentData != nullptr) delete currentData;

			// Load CSV data
				currentData = new CSV(choice[0]);

			// Update GUI to match the parameters of the new CSV file
			GuiState.numberOfDataTypes = currentData->getNumberOfDataTypes();
			GuiState.numberOfTimePoints = currentData->getNumberOfTimePoints();
			GuiState.dataFields = currentData->getOrderedData();
			GuiState.dataFieldsEnabled = new bool[GuiState.numberOfDataTypes];
			GuiState.sceneOpen = true;

			// Start off with all data disabled
			memset(GuiState.dataFieldsEnabled, 0, GuiState.numberOfDataTypes);
		}
		}

		// If timeline is set to 'play', add to the tick total and increment the timeline position
		if (GuiState.isPlaying) {
			tickTotal += abs(GuiState.playbackSpeed) * dTime; // abs value to allow for backwards playing

			while (tickTotal > 1){
				GuiState.timelinePosition += Utils::sign(GuiState.playbackSpeed);
				--tickTotal;
				// just for testing
				// for (int i = 0; i < GuiState.numberOfDataTypes; i++){
				// 	if (*(GuiState.dataFieldsEnabled + i)) {
				// 		std::cout << '\r' << 
				// 			GuiState.dataFields[i] << " at " << 
				// 			GuiState.timelinePosition << " : " << 
				// 			currentData->getData(i, GuiState.timelinePosition);
				// 	}
				// } // todo:: delete
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
		// Handle resize of window, and update to camera settings through ui, both of which need a new camera view matrix
		//

		// If user dragged window size around
		if (frameSizeChanged) {
			glfwGetFramebufferSize(window, &::WorldState.windowX, &::WorldState.windowY);

			if ((::WorldState.windowX > 1) && (::WorldState.windowY > 1)) { // Minimized window triggers this, causing crash when trying to calculate mv matrix for 0 size window
				WorldState.rendererX = WorldState.windowX;
				WorldState.rendererY = WorldState.windowY / 2;

				// Setup renderer to only draw in top half of screen
				glViewport(0, WorldState.windowY - WorldState.rendererY, WorldState.windowX, WorldState.rendererY);

				GuiState.cameraSettingsChanged = true; // force new perpective matrix
				frameSizeChanged = false;
			}
		}

		// If camera settings changed, build new view matrix to reflect these changes
		if (GuiState.cameraSettingsChanged) {
			renderer->getMainCamera()->setFOV(CAMERA_DEFAULT_FOV + GuiState.FOV);
			renderer->getMainCamera()->setFarClipPlane(GuiState.farClipPlane);
			GuiState.cameraSettingsChanged = false;
		}

		// Adjust brightness based on brightness slider
		WorldState.ambientLight[0] = originalAmbientLight[0] + GuiState.brightness;
		WorldState.ambientLight[1] = originalAmbientLight[1] + GuiState.brightness;
		WorldState.ambientLight[2] = originalAmbientLight[2] + GuiState.brightness;

		// Draw renderable elements
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderer->tick(dTime);

		// Draw GUI elements
		drawUI(GuiState);

		// Current frame finished

		glfwSwapBuffers(window);
		glfwPollEvents();

		t2 = std::chrono::steady_clock::now();
		dTime = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1000000.0f; // Microsecond conversion into fraction of second
	}

	delete renderer; // Deletes internal scene data

end_program:

	// Cleanup

	glfwDestroyWindow(window);

	cleanup();

	return 0;
}