#define STB_IMAGE_IMPLEMENTATION // Needed for stbi image loading

#define WINDOW_TITLE "Racetrack Stats"

// #ifdef MAKE_DLL
// 	#define DLL_EXPORT __declspec(dllexport)
// #else 
// 	#define DLL_EXPORT ""
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
#include <imgui_internal.h>

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

// State of the scene, used by the renderer
_WorldState WorldState = { 
	WINDOW_DEFAULT_X,    // Starting window width
	WINDOW_DEFAULT_Y,    // Starting window height
	{0.0f, 0.0f, 0.0f},  // vec3 ambient color for the scene
	nullptr,		     // (string) path to root of executable (set in main)
	nullptr				 // (string) path to root of the mesh and texture data for mosport
};

// Internal state variables
const static char* exeLocation;
const static char* trackDataLocation;

static CSV* currentData;

static bool frameSizeChanged = false;

ImGuiIO* imguiIO;

GLFWwindow* g_Window = NULL; // Hacked the glfw implementation of ImGui a bit

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
}

void cleanup() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void createWindow(GLFWwindow* parentWindow, ImGuiContext* parentContext) {
	GLFWwindow* window = glfwCreateWindow(300, 200, "ASDF", NULL, NULL);

	glfwMakeContextCurrent(window);

	ImGui_ImplGlfw_InitForOpenGL(window, false);

	ImGuiContext* imguiContext = ImGui::CreateContext(parentContext->Font->ContainerAtlas);

	ImGui::SetCurrentContext(imguiContext);

	// ImGui_ImplOpenGL3_Init("#version 330");

	std::cout << "New Window Created!" << std::endl;

	while (!glfwWindowShouldClose(window)) {
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("TEST!");
		ImGui::Button("asdf");
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		// glfwSwapBuffers(parentWindow);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);

	// imguiContext->IO = ImGuiIO(); // destroying the context destoys the shared io state... for some reason, so make a dummy IO handle
	ImGui::DestroyContext(imguiContext);

	glfwMakeContextCurrent(parentWindow);

	ImGui::SetCurrentContext(parentContext);
	ImGui_ImplGlfw_InitForOpenGL(parentWindow, false);

	std::cout << "New Window Destroyed..." << std::endl;
}

int main(int argc, char** argv) {

	glfwInit();	

	GLFWwindow* window1 = glfwCreateWindow(400, 400, "WINDOW 1", NULL, NULL);
	GLFWwindow* window2 = glfwCreateWindow(400, 400, "WINDOW 2", NULL, NULL);

	// WINDOW 1
	glfwMakeContextCurrent(window1);
	
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD\n";
	}

	ImGuiContext* ctx1 = ImGui::CreateContext();
	ImGui::SetCurrentContext(ctx1);
	ImGuiIO& IO1 = ImGui::GetIO();

	ImGui_ImplGlfw_InitForOpenGL(window1, false);
	ImGui_ImplOpenGL3_Init("#version 330");

	ImGui::SetCurrentContext(NULL);

	// WINDOW 2
	glfwMakeContextCurrent(window2);
	
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD\n";
	}

	ImGuiContext* ctx2 = ImGui::CreateContext();
	ImGui::SetCurrentContext(ctx2);
	ImGuiIO& IO2 = ImGui::GetIO();

	ImGui_ImplGlfw_InitForOpenGL(window2, false);
	ImGui_ImplOpenGL3_Init("#version 330");

	ImGui::SetCurrentContext(NULL);

	// std::cout << "Made contexts: 1: " << ctx1 << ", 2: " << ctx2 << std::endl;

	while (!(glfwWindowShouldClose(window2) && glfwWindowShouldClose(window2))) {

		// if (glfwGetKey(window1, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		// 	glfwSetWindowShouldClose(window1, true);
		// 	glfwDestroyWindow(window1);
		// }

		if (glfwGetKey(window2, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window2, true);
			glfwDestroyWindow(window2);
		}

		if (!glfwWindowShouldClose(window1)) {
			// Window 1
			glfwMakeContextCurrent(window1);
			ImGui::SetCurrentContext(ctx1);

			g_Window = window1;

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::Begin("WINDOW 1");
			ImGui::Button("asdf");
			ImGui::End();

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			glfwSwapBuffers(window1);

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		if (!(ctx2->IO.Fonts->IsBuilt())) {
			ctx2->IO.Fonts = ctx1->IO.Fonts;
		}

		if (!glfwWindowShouldClose(window2)) {
			// Window 2
			glfwMakeContextCurrent(window2);
			ImGui::SetCurrentContext(ctx2);

			g_Window = window2;

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::Begin("WINDOW 2");
			ImGui::Button("fdasfasdff");
			ImGui::End();

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			glfwSwapBuffers(window2);

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		glfwPollEvents();
	}

	return 0;


	exeLocation = Utils::getFileInfo(*argv).directory.c_str();
	std::cout << "Launching racetracks stats... (" << exeLocation << ")" << std::endl << std::endl;

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

	// Initialize renderer
	Renderer* renderer = new Renderer(window);

	glfwSetWindowPos(window, 40, 40); // Not necessary, just makes the window appear in a consistent spot

	// Setup renderer to only draw in top half of screen
	glViewport(0, WorldState.windowY / 2, WorldState.windowX, WorldState.windowY);

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

	//
	// Main loop
	// 

	bool windowDoStop = false;

	while (!glfwWindowShouldClose(window)) {
		if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
			createWindow(window, imguiContext);
		}

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
				glViewport(0, WorldState.windowY / 2, WorldState.windowX, WorldState.windowY);

				GuiState.cameraSettingsChanged = true; // force new perpective matrix
				frameSizeChanged = false;
			}
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