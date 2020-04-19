#define STB_IMAGE_IMPLEMENTATION // Needed for stbi image loading

#define RACETRACK_STATS_APP // guards some includes, in case some of the files are being used on their own

#define WINDOW_TITLE "Racetrack Stats"

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

// #if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
// #include <wglext.h>
// #endif // Windows
// bool (*swapFunction)(int) = (bool (*)(int)) wglGetProcAddress("wglGetSwapIntervalEXT");

//
// Data shared with other files
//

_GuiState GuiState;
ImGuiIO* imguiIO;

CSV* currentData;

Utils::CSVDataDisplaySettings displayData;

_WorldState WorldState = { // State of the scene, used by the renderer and the gui
	WINDOW_DEFAULT_X,    // Starting window width
	WINDOW_DEFAULT_Y,    // Starting window height
	WINDOW_DEFAULT_X,    // Proper values for renderer w h get filled in main
	WINDOW_DEFAULT_Y,     
	{0.0f, 0.0f, 0.0f},  // vec3 ambient color for the scene
	nullptr,		     // (string) path to root of executable (set in main)
	nullptr				 // (string) path to root of the mesh and texture data for mosport
};

//
// Internal variables
//

static Renderer* renderer;

const char* exeLocation;
const char* trackDataLocation;

static const char* displayDataConfigFileName = "display.config";

static bool frameSizeChanged = false;

static bool mouseMoved = false;
static double dMouseX = 0;
static double dMouseY = 0;
static float dMouseScrollY = 0.0f;

static double tickTotal = 0;

// Handler for shortcuts used to navigate the ui
void keyPressCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// Escape to close the program
	// TODO: remove in final?
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

	// Spacebar to play/pause
	if (key ==  GLFW_KEY_SPACE && action == GLFW_PRESS) GuiState.isPlaying = !GuiState.isPlaying;

	// Arrow keys to nudge timeline
	int timelineMax = currentData == nullptr ? 0 : currentData->getNumberOfTimePoints();
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)   GuiState.timelinePosition = Utils::clamp(GuiState.timelinePosition - GuiState.tickSkipAmount, 0, timelineMax);
	if (key ==  GLFW_KEY_RIGHT && action == GLFW_PRESS) GuiState.timelinePosition = Utils::clamp(GuiState.timelinePosition + GuiState.tickSkipAmount, 0, timelineMax);

	// Number keys to switch between cameras
	if (key == GLFW_KEY_1 && action == GLFW_PRESS) renderer->setMainCamera("Follow");
	if (key == GLFW_KEY_2 && action == GLFW_PRESS) renderer->setMainCamera("Car");
	if (key == GLFW_KEY_3 && action == GLFW_PRESS) renderer->setMainCamera("Overhead");
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

void mouseScrollCallback(GLFWwindow* window, double scrollX, double scrollY) {
	dMouseScrollY += scrollY;
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
		std::cerr << "ERROR: Failed to initialize GLAD\n";
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
	glfwSetScrollCallback(window, mouseScrollCallback);


	// For personal convenience when working, this is defined on one of my machines
	bool onMSI = std::getenv("MSI") != nullptr;
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
	imguiIO = &ImGui::GetIO();
	imguiIO->FontGlobalScale = 1.3;

	// ::GuiState.io->FontGlobalScale = 1.3;
	setGuiOptionsToDefault();
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

	std::cout << "Selected " << selectedScene << std::endl;

	glfwSetWindowPos(window, 40, 40); // Not necessary, just makes the window appear in a consistent spot

	// Default size of renderer window
	WorldState.rendererX = WorldState.windowX;
	WorldState.rendererY = WorldState.windowY / 2;

	// Setup renderer to only draw in top half of screen
	glViewport(0, WorldState.windowY - WorldState.rendererY, WorldState.windowX, WorldState.rendererY);

	// Initialize renderer
	renderer = new Renderer(window);

	// Draw one frame of GUI to look clean while loading
	drawUI();
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
	double t0, t1;

	double dTime = 0;
	double seconds0 = glfwGetTime();

	// Used to calculate timeline playing
	double tickTotal = 0;

	// Parent transform for the whole BMW
	Transform* BMW_transform = new Transform();

	for (Object* o : renderer->objects) {
		if (Utils::hasStart(o->mesh->getMeshName(), "BMW")) { // If object is from BMW obj file
			o->transform->setParent(BMW_transform);
		}
	}

	//
	// Set up default cameras
	//
	Camera* followCam = new Camera();
	Camera* carCam = new Camera();
	Camera* overheadCam = new Camera(Orthographic);

	renderer->registerCamera("Follow", followCam);
	renderer->registerCamera("Car", carCam);
	renderer->registerCamera("Overhead", overheadCam);

	// Use an intermediary transform between the car and camera so rotation is simple
	Transform* followCamLocator = new Transform();
	Transform* carCamLocator = new Transform();
	Transform* overheadCamLocator = new Transform();

	followCam->transform->setParent(followCamLocator);
	carCam->transform->setParent(carCamLocator);
	overheadCam->transform->setParent(overheadCamLocator);

	followCamLocator->setParent(BMW_transform);
	carCamLocator->setParent(BMW_transform);
	overheadCamLocator->setParent(BMW_transform);

	carCamLocator->setTranslation({ 0.3665, 1.1, -0.1 }); // Center camera approximately where the driver's head is
	carCamLocator->setRotation(Utils::PI, { 0, 1, 0 });

	followCam->transform->setTranslation({ 0, 0, 7 });
	followCamLocator->rotate(Utils::PI, { 0,1,0 });

	overheadCamLocator->rotate(Utils::PI, { 0,1,0 });
	overheadCamLocator->rotate(Utils::PI/2, { 1,0,0 });

	renderer->setMainCamera("Follow");

	// Load config file for CSV data display
	displayData = Utils::loadDisplaySettings(std::string(WorldState.projectRoot) + DIRECTORY_SEPARATOR + displayDataConfigFileName);

	// TODO: This is for testing, approximately at start of course
	BMW_transform->setTranslation({ -270.326, 1.50942, -753.757 });

	//
	// Main loop
	// 

	while (!glfwWindowShouldClose(window)) {
		t0 = glfwGetTime();

		// Calculate fps of last frame, pass to gui
		if ((t0 - seconds0) > 0.1) { // Dont need to do this every frame, instead only every .1 seconds
			GuiState.fps = 1/dTime;
			seconds0 = t0;
		}

		// TODO: just for testing
		float rSpeed = 1;
		float tSpeed = 3;

		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
			BMW_transform->rotate(dTime * rSpeed, glm::vec3(0, 1, 0));
		}

		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			BMW_transform->rotate(dTime * -rSpeed, glm::vec3(0, 1, 0));
		}

		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
			BMW_transform->translate(dTime * tSpeed * BMW_transform->forward());
		}

		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
			BMW_transform->translate(dTime * -tSpeed * BMW_transform->forward());
		}

		//
		// Process user input
		//

		// 'Zoom' camera if scroll wheel was used
		if (dMouseScrollY != 0.0f) {

			double xPos, yPos;
			glfwGetCursorPos(window, &xPos, &yPos);

			// Only zoom camera if mouse is on the viewport. (TODO: change this in the future, this is pretty crude)
			if ((yPos < WorldState.rendererY)) {
				Camera* mainCam = renderer->getMainCamera();

				// Different behavior for ortho and persp cams

				const float orthoZoomSpeed = -0.001f;
				const float perspZoomSpeed = -0.8f;

				if (mainCam == overheadCam) {
					mainCam->setSize(mainCam->getSize() + orthoZoomSpeed * dMouseScrollY); // TODO: This should also probably control the near/far plane on this camera
				}
				else if (mainCam == followCam) { 
					mainCam->transform->translate({ 0, 0, perspZoomSpeed * dMouseScrollY });
				} // No real reason to 'zoom' in cockpit view
			}

			dMouseScrollY = 0;
		}

		// Rotate current camera/locator if user dragged right click
		if (mouseMoved) {
			if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) {
				Camera* mainCam = renderer->getMainCamera();

				Transform* t = nullptr;

				if (mainCam == carCam) t = carCam->transform;
				if (mainCam == followCam) t = followCamLocator;
				if (mainCam == overheadCam) t = overheadCamLocator;

				if (t != nullptr) {
					t->rotate(dTime * 0.01f * GuiState.mouseSensitivity * -dMouseX, { 0,1,0 });
					t->rotate(dTime * 0.01f * GuiState.mouseSensitivity * dMouseY, t->right());
				}
			}

			mouseMoved = false;
		}

		// TODO: Ignorable crash when new CSV loaded?

		// If load new scene clicked
		if (GuiState.selected_menu_File_Open) {
			// Launch open file dialog
			pfd::open_file fd = pfd::open_file::open_file("Open CSV File", "", {"All Files", "*.csv" }, false);
			std::vector<std::string> choice = fd.result();

			// Returned list is empty if user cancelled window, else it contains what the user selected

			if (choice.size() != 0) { 
				// Clean up old data if necessary 
				if (currentData != nullptr) delete currentData;

				// Load CSV data
				currentData = new CSV(choice[0]);

				// Update GUI to match the parameters of the new CSV file
				GuiState.dataFields = currentData->getOrderedData();
				GuiState.sceneOpen = true;

				organizeData();
			}
		}

		// If load new config file clicked
		if (GuiState.selected_menu_File_LoadConfig) {
			pfd::open_file fd = pfd::open_file::open_file("Open Config File", "", {"All Files", "*.config" }, false);
			std::vector<std::string> choice = fd.result();

			if (choice.size() != 0) {
				displayData = Utils::loadDisplaySettings(choice[0]);
				organizeData();
			}
		}

		// If reload config clicked
		if (GuiState.selected_menu_File_ReloadConfig) {
			std::string path = displayData.path;
			displayData = Utils::loadDisplaySettings(path);
			organizeData();
		}

		static int lastTimelinePos = 0;

		// 
		// If there is CSV data loaded, handle the timeline, and any data that is being interpreted
		//
		if (currentData) {
			// If timeline is set to 'play', add to the tick total and increment the timeline position
			if (GuiState.isPlaying) {
				tickTotal += GuiState.playbackSpeed * dTime; 

				int tickIncrease = (int) tickTotal; // whole number portion of the tick total we can add to the timeline
				tickTotal -= tickIncrease; // Overflow will accumulate and contribute to future ticks

				GuiState.timelinePosition = Utils::clamp(GuiState.timelinePosition + tickIncrease, 0, currentData->getNumberOfTimePoints());
			}

			float vectorScale;
			glm::mat4 bmw_transformation = BMW_transform->getMatrix();

			for (DataState& d : GuiState.dataStates) {
				if (d.enabled && d.type == DataType::Vector) {
					vectorScale = currentData->getDataAsFloat(d.vector.dataField, GuiState.timelinePosition);

					if (vectorScale == 0.0f) continue;

					vectorScale = vectorScale / d.max;

					renderer->addLine(
						BMW_transform->position() + d.vector.origin,
						bmw_transformation *  glm::vec4(vectorScale * d.vector.direction, 1),
						d.vector.color,
						true
					);
				}
			}
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
			followCam->setFOV(CAMERA_DEFAULT_FOV + GuiState.FOV);
			followCam->setFarClipPlane(GuiState.farClipPlane);

			carCam->setFOV(CAMERA_DEFAULT_FOV + GuiState.FOV);
			carCam->setFarClipPlane(GuiState.farClipPlane);

			overheadCam->setFarClipPlane(GuiState.farClipPlane);

			GuiState.cameraSettingsChanged = false;
		}

		// Adjust brightness based on brightness slider
		WorldState.ambientLight[0] = originalAmbientLight[0] + GuiState.brightness;
		WorldState.ambientLight[1] = originalAmbientLight[1] + GuiState.brightness;
		WorldState.ambientLight[2] = originalAmbientLight[2] + GuiState.brightness;

		// Draw renderable elements
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// TODO: Optimization, renderer and ui draws can happen in parrallel, if the beginning and ending code of drawUI is moved here instead
		// All other ImGui calls in that function just store ui draw data, but don't actually use any opengl calls (I believe)

		float r0 = glfwGetTime();
		renderer->tick(dTime);
		float renderTime = glfwGetTime() - r0;

		r0 = glfwGetTime();
		// Draw GUI elements
		drawUI();
		float UITime = glfwGetTime() - r0;

		float renderPercentage = renderTime / (renderTime + UITime);
		float UIPercentage = UITime / (renderTime + UITime);

		static float s0 = glfwGetTime();
		if ((glfwGetTime() - s0) > 0.1) {
			std::cout << "\r                                                                                              "; // TODO: erase stdout line?
			std::cout << '\r' << "[" << 1.0f/(renderTime + UITime) << " fps] " << "Render Time: " << renderTime << " (" << renderPercentage << "%)" << " | UI Draw Time: " << UITime << " (" << UIPercentage << "%)";
			s0 = glfwGetTime();
		}

		// Current frame finished

		glfwSwapBuffers(window);
		glfwPollEvents();

		t1 = glfwGetTime();
		dTime = t1 - t0;
	}

	delete renderer; // Deletes internal scene data

end_program:

	// Cleanup

	glfwDestroyWindow(window);

	cleanup();

	return 0;
}