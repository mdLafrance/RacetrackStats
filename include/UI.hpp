#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include <iostream>

#include <imgui.h>

#include <Texture.h>
#include <WorldState.h>
#include <CSV.h>
#include <Utils.h>

#define UI_DEFAULT_TIMELINE_CONTROLS_HEIGHT 150

struct _GuiState;

// Global variables, initialized in main
extern _GuiState GuiState; 
extern _WorldState WorldState;
extern ImGuiIO* imguiIO; 
extern CSV* currentData;
extern Utils::CSVDataDisplaySettings displayData;

enum class DataType { Vector, Graph };

struct DataState {
	DataType type;

	Utils::CSVgraph graph;
	Utils::CSVvector vector;

	float max = 0.0f;
	float min = 0.0f; 

	bool enabled = false;

	std::string toString() {
		std::string s;
		s += (type == DataType::Vector ? "Vector " + this->vector.dataField : "Graph " + this->graph.dataField);
		s += ' ';

		s += (this->enabled ? "[ENABLED]" : "[DISABLED]");

		if (this->type == DataType::Graph) {
			s += ' ';
			s += "Min/Max : (" + std::to_string(this->min) + ", " + std::to_string(this->max) + ")";
		}

		return s;
	}

	DataState(const Utils::CSVvector& v) {
		this->type = DataType::Vector;
		this->vector = v;
		this->enabled = false;

	}

	DataState(const Utils::CSVgraph& g) {
		this->type = DataType::Graph;
		this->graph = g;
		this->enabled = false;
	}

	DataState(const DataState& other) {
		this->type = other.type;

		if (this->type == DataType::Vector) {
			this->vector = other.vector;
		}

		if (this->type == DataType::Graph) {
			this->graph = other.graph;
		}

		this->max = other.max;
		this->min = other.min;
		this->enabled = other.enabled;
	}

	~DataState() {};
};

struct _GuiState {
	// Global states
	bool sceneOpen = false;
	int padding = 17;

	Texture* mapTexture;

	// Textures for the custom buttons; play/pause button, skip buttons
	// [Normal, Hovered, Clicked]
	Texture* playButtonTextures[3];
	Texture* pauseButtonTextures[3];
	Texture* skipButtonTextures[3];

	double fps = 0.0; 

	bool cameraSettingsChanged = false;

	float mouseSensitivity = 1.0f;
	
	// Render states
	float FOV = 0.0f;
	float nearClipPlane = 0.0f;
	float farClipPlane = 100.0f;
	float brightness = 0.0f;

	bool lineWidthChanged = false;
	float lineWidth = 1;
	float glLineWidthRange[2];

	// CSV data fields
	std::vector<std::string> dataFields; // The data fields which are both present in the CSV, and in the config file
	std::vector<DataState> dataStates;

	int dataBufferSize = 100;

    // Timeline states
    int timelinePosition = 0;
	bool isPlaying = false;
	float playbackSpeed = 1.0f; // Ticks per second
	int tickSkipAmount = 1; // Amount of ticks that should be skipped with arrows

	// Menu bar state
	bool selected_menu_File = false;
	bool selected_menu_File_Open = false;
	bool selected_menu_Options = false;
	bool selected_menu_File_LoadConfig = false;
	bool selected_menu_File_ReloadConfig = false;
	bool doShowMap = false;
	bool doShowFPSCounter = true;

	~_GuiState() {
		delete mapTexture;
		
		for (int i = 0; i < 3; i++) { // Can't delete[] since the arrays aren't on heap
			delete *(playButtonTextures + i);
			delete *(pauseButtonTextures + i);
			delete *(skipButtonTextures + i);
		}
	}
};

void organizeData() {
	GuiState.dataStates.clear();

	for (Utils::CSVvector v : displayData.vectors) {
		if (!currentData->hasData(v.dataField)) {
			std::cerr << "ERROR: Data field defined in config missing from CSV file: " << v.dataField << std::endl;
			continue;
		}

		DataState d(v);

		currentData->getDataMinMax(v.dataField, &d.min, &d.max);

		GuiState.dataStates.push_back(d);
	}

	for (Utils::CSVgraph g : displayData.graphs) {
		if (!currentData->hasData(g.dataField)) {
			std::cerr << "ERROR: Data field defined in config missing from CSV file: " << g.dataField << std::endl;
			continue;
		}

		DataState d(g);

		currentData->getDataMinMax(g.dataField, &d.min, &d.max);

		GuiState.dataStates.push_back(d);
	}

	for (DataState d : GuiState.dataStates) {
		std::cout << d.toString() << std::endl;
	}
}

void setGuiOptionsToDefault() {
	GuiState.doShowFPSCounter = true;
	GuiState.doShowMap = false;

	GuiState.cameraSettingsChanged = true;

	GuiState.nearClipPlane = 0.5f;
	GuiState.farClipPlane = 1000.0f;
	GuiState.brightness = 0.0f;
	GuiState.FOV = 0.0f;

	GuiState.dataBufferSize = 100;

	GuiState.mouseSensitivity = 20.0f;

	GuiState.lineWidth = 1.0f;
	GuiState.lineWidthChanged = true;
}

// NOTE: Bug within ImGui causes multiple ImageButtons to not receive any clicks (documented bug)
// So just made an ImageButton where this... doesn't happen
bool ImageButton2(Texture** textures, const ImVec2& cursorPos, const ImVec2& buttonSize, const bool& flipTexture = false) {
	/*
		Draw an image, and handle mouse click on this texture to mimic image button
		-textures should be the pointer to an array of 3 textures in order of 'Normal', 'Hovered', and 'Clicked' appearance.
		-Cursor Pos is the top left of the button in global app coordinates
		-Button size and flip texture fairly straight forward

		Usage is the same as ImGui::ImageButton

		NOTE: CursorPos refers to the imGui 'paintbrush' position in app coordinates, not the mouse position
	*/
	assert(textures != nullptr);

	enum ButtonState { Normal = 0, Hovered = 1, Clicked = 2 };

	ImVec2 mousePos = ImGui::GetMousePos();

	ImVec2 boundsX(cursorPos[0], cursorPos[0] + buttonSize[0]);
	ImVec2 boundsY(cursorPos[1], cursorPos[1] + buttonSize[1]);

	ButtonState bs;

	if ((boundsX[0] <= mousePos[0] && mousePos[0] <= boundsX[1]) && (boundsY[0] <= mousePos[1] && mousePos[1] <= boundsY[1])) { // If mouse within bounds of button
		if (imguiIO->MouseDown[0]) { // If left click is being held
			bs = Clicked;
		}
		else {
			bs = Hovered;
		}
	}
	else { // Not hovered
		bs = Normal;
	}

	// Create image with correct button image
	ImGui::Image((void*)(*(textures + (int)bs))->getID(), buttonSize, ImVec2(), flipTexture ? ImVec2(-1,-1) : ImVec2(1, 1));

	return (bs == Clicked) && ImGui::IsMouseClicked(0, false); // Is the mouse clicking on the button, and did the click happen this frame?
}

void drawUI() {
	// Main menu drop down bar
	int menuBarHeight = imguiIO->FontGlobalScale * 10 + 12; // eyeballing this a bit lol

	int X, Y, halfX, halfY;
	X = WorldState.windowX;
	Y = WorldState.windowY;
	halfX = X / 2;
	halfY = Y / 2;

	int w, h; // Used to hold various button and image widths and heights later on

	// Data that is dependant on whether or not CSV data is loaded
	int timelineMax = currentData == nullptr ? 0 : currentData->getNumberOfTimePoints() - 1;

	// NOTE: Spent time looking for the right style element color for this, but ended up just eyedropping the color
	ImVec4 bgColor = ImVec4(0.056, 0.056, 0.056, 1.0f);// ImGui::GetStyleColorVec4(ImGuiCol_TitleBg); // Cache background color for use with the icon buttons later

	// Menu buttons are toggled, but we want them to act as a button, so always flip them back to false 
	GuiState.selected_menu_File_Open = false;
	GuiState.selected_menu_File_LoadConfig = false;
	GuiState.selected_menu_File_ReloadConfig = false;

	if (ImGui::BeginMainMenuBar()) {

		if (ImGui::BeginMenu("File", &GuiState.selected_menu_File)) {
			ImGui::MenuItem("Open CSV", NULL, &GuiState.selected_menu_File_Open);
			ImGui::MenuItem("Load Data Config File", NULL, &GuiState.selected_menu_File_LoadConfig);
			ImGui::MenuItem("Reload Config", NULL, &GuiState.selected_menu_File_ReloadConfig);
			ImGui::EndMenu();
			ImGui::Separator();
			ImGui::Separator();
		}

		if (ImGui::BeginMenu("Options", &GuiState.selected_menu_Options)) {

			// GUI OPTIONS

			ImGui::Separator();
			ImGui::Separator();

			ImGui::Text("GUI Options");
			ImGui::Separator();
			// ImGui::MenuItem("Show Map", NULL, &GuiState.doShowMap);
			ImGui::MenuItem("Show FPS Counter", NULL, &GuiState.doShowFPSCounter);
			ImGui::Text("Font Size");
			ImGui::SameLine(300, 0);
			ImGui::PushItemWidth(100);
			ImGui::InputFloat("##Font size", &imguiIO->FontGlobalScale, 0, 0, "%.1f");

			imguiIO->FontGlobalScale = Utils::clamp(imguiIO->FontGlobalScale, 0.0f, 3.0f); // things start getting weird above 3 font lol

			ImGui::Text("Camera Mouse Sensitivity");
			ImGui::SameLine(300, 0);
			ImGui::PushItemWidth(100);
			ImGui::SliderFloat("##Camera Mouse Sensitivity", &GuiState.mouseSensitivity, 0.0f, 200.0f);

			ImGui::Text("Data Points On Graph");
			ImGui::SameLine(300, 0);
			ImGui::InputInt("## Data Points On Graph", &GuiState.dataBufferSize, 0, 0);

			if (currentData) GuiState.dataBufferSize = Utils::clamp(GuiState.dataBufferSize, 0, currentData->getNumberOfTimePoints());

			ImGui::Separator();
			ImGui::Separator();

			// RENDER OPTIONS

			ImGui::Text("Render Options");
			ImGui::Separator();

			const float FOVbounds = 1.0f;
			ImGui::Text("FOV -/+");
			ImGui::SameLine(300, 0);
			if (ImGui::SliderFloat("##FOV", &GuiState.FOV, -FOVbounds, FOVbounds)) GuiState.cameraSettingsChanged = true;

			// ImGui::Text("Near Clip Plane");
			// ImGui::SameLine(300, 0);
			// if (ImGui::InputFloat("##Near Clip Plane", &state.nearClipPlane, 0, 0, "%.2f")) state.cameraSettingsChanged = true;

			ImGui::Text("Far Clip Plane");
			ImGui::SameLine(300, 0);
			if (ImGui::InputFloat("##Far Clip Plane", &GuiState.farClipPlane, 0, 0, "%.2f")) GuiState.cameraSettingsChanged = true;

			ImGui::Text("Brightness +/-");
			ImGui::SameLine(300, 0);
			ImGui::SliderFloat("##Brightness", &GuiState.brightness, -1, 1);

			// gl may not enable line width changes, in which case the max is 1
			if (GuiState.glLineWidthRange[1] > 1) {
				ImGui::Text("Line Width");
				ImGui::SameLine(300, 0);
				if (ImGui::SliderFloat("##Line Widdth", &GuiState.lineWidth, GuiState.glLineWidthRange[0], Utils::clamp(GuiState.glLineWidthRange[1], 1.0f, 10.0f))) GuiState.lineWidthChanged = true;
			}

			ImGui::Separator();
			ImGui::Separator();

			ImGui::PushItemWidth(300);
			if (ImGui::Button("Reset to Defaults")) setGuiOptionsToDefault();

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	} // Main menu drop down bar

	// Data panel
	ImGui::SetNextWindowSize(ImVec2(X, halfY - UI_DEFAULT_TIMELINE_CONTROLS_HEIGHT));
	ImGui::SetNextWindowPos(ImVec2(0, halfY), 0, ImVec2(0, 0));
	ImGui::Begin("Data", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);


	// Dropdown to enable or disable data type display
	if (currentData != nullptr) {
		std::vector<std::string> currentDisplayData;

		for (Utils::CSVgraph g : displayData.graphs) {
			if (currentData->hasData(g.dataField)) {
				currentDisplayData.push_back(g.dataField);
			}
		}

		if (ImGui::CollapsingHeader("Display Data Types")) {
			// 'All' and 'None' buttons

			// if (ImGui::Button("All")) {
			//     memset(state.dataFieldsEnabled, 1, state.dataFields.size());
			// }

			// ImGui::SameLine(0, 10);

			if (ImGui::Button("All")) {
				for (DataState& d : GuiState.dataStates) {
					d.enabled = true;
				}
			}

			ImGui::SameLine(0, 10);

			if (ImGui::Button("None")) {
				for (DataState& d : GuiState.dataStates) {
					d.enabled = false;
				}
			}

			for (DataState& data : GuiState.dataStates) {
				if (data.type == DataType::Vector) {
					ImGui::Checkbox(("[Vector] " + data.vector.dataField).c_str(), &data.enabled);
				}

				if (data.type == DataType::Graph) {
					ImGui::Checkbox(("[Graph]  " + data.graph.dataField).c_str(), &data.enabled);
				}
			}
		}
	} // Data type dropdown


	//
	// Draw the graphs and buffer them with data
	//

	float* bufferedData = new float [GuiState.dataBufferSize];

	for (DataState& data : GuiState.dataStates) {
		if (data.enabled) {
			if (data.type == DataType::Graph) {
				// TODO: Major optimization here, this data should be cached and selectively replaced instead of being completely fetched every frame

				// Buffer data
				currentData->getBatchDataAsFloat(
					data.graph.dataField, 
					GuiState.timelinePosition,
					Utils::clamp(GuiState.timelinePosition + GuiState.dataBufferSize, 0, currentData->getNumberOfTimePoints()), 
					bufferedData
				);

				// Draw graph
				// TODO: Use third party imgui_plot library for better looking graphs?
				ImGui::PlotLines(
					data.graph.dataField.c_str(), 
					bufferedData, 
					GuiState.dataBufferSize, 
					0, 
					std::to_string(currentData->getDataAsFloat(data.graph.dataField, GuiState.timelinePosition)).c_str(), 
					data.min, 
					data.max, 
					ImVec2(X - 150, 40)
				);
			}
		}
	}

	delete[] bufferedData;

	ImGui::End(); // Data Panel

	// Timeline panel
	ImVec2 timelinePanelPosition(0, Y - UI_DEFAULT_TIMELINE_CONTROLS_HEIGHT);

	ImGui::SetNextWindowSize(ImVec2(X, UI_DEFAULT_TIMELINE_CONTROLS_HEIGHT));
	ImGui::SetNextWindowPos(timelinePanelPosition, 0, ImVec2(0, 0));
	ImGui::Begin("Timeline Panel", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

	ImGui::PushItemWidth(X - GuiState.padding);
	ImGui::SliderInt("##Timeline", &GuiState.timelinePosition, 0, timelineMax);

	//
	// Timeline control buttons
	//

	int buttonState = 0; // 0 is normal, 1 is hovered, 2 is pressed and hovered. 
	Texture* buttonTexture;

	// Determine the right color for the button

	// Params for timeline controllers
	ImVec2 mousePos = ImGui::GetMousePos();

	ImVec2 boundsX, boundsY, cursorPos;

	// Following values are used to calculate the shapes and positions of the buttons
	const int playButtonDimensions = 50;
	const int arrowButtonDimensions = 40;
	const int halfArrowButtonDimensions = arrowButtonDimensions / 2;

	const int halfDiff = (playButtonDimensions - arrowButtonDimensions) / 2;

	const int arrowButtonOffset = 60;

	const int timeLineButtonLocalVerticalAlign = UI_DEFAULT_TIMELINE_CONTROLS_HEIGHT / 2 - 10;

	ImVec2 pbDimensions = ImVec2(playButtonDimensions, playButtonDimensions); // play button
	ImVec2 bfDimensions = ImVec2(arrowButtonDimensions, arrowButtonDimensions); // back forward

	//
	// Play Button
	//
 
	Texture** ppButton = GuiState.isPlaying ? GuiState.pauseButtonTextures : GuiState.playButtonTextures; //pause/play button, which texture set should be used?

	// Cursor is where ImGui will draw the item
	cursorPos = {(float) (X - playButtonDimensions) / 2, (float)timeLineButtonLocalVerticalAlign };
	ImGui::SetCursorPos(cursorPos);

	// NOTE: need to add the position of the parent panel to convert into 'global' app pixel coordinates, since cursor is in 'local' panel window coordinates
	if (ImageButton2(ppButton, cursorPos + timelinePanelPosition, pbDimensions, false)) {
		// Flip the button to other state
		if (GuiState.isPlaying) {
			GuiState.isPlaying = false;
		}
		else {
			GuiState.isPlaying = true;
			GuiState.timelinePosition += Utils::sign(GuiState.playbackSpeed); // Increment the timeline on the frame when the play button is pressed
		}
	}

	//
	// Back and forward buttons
	//

	// Back Button
	cursorPos = {(float) halfX - halfArrowButtonDimensions - arrowButtonOffset, (float) timeLineButtonLocalVerticalAlign + halfDiff };
	ImGui::SetCursorPos(cursorPos);

	if (ImageButton2(GuiState.skipButtonTextures, cursorPos + timelinePanelPosition, bfDimensions, true)) GuiState.timelinePosition -= GuiState.tickSkipAmount;

	// // Forward button
	cursorPos = {(float) halfX - halfArrowButtonDimensions + arrowButtonOffset, (float) timeLineButtonLocalVerticalAlign + halfDiff };
	ImGui::SetCursorPos(cursorPos);

	if (ImageButton2(GuiState.skipButtonTextures, cursorPos + timelinePanelPosition, bfDimensions)) GuiState.timelinePosition += GuiState.tickSkipAmount;

	// Clamp timeline within bounds
	GuiState.timelinePosition = Utils::clamp(GuiState.timelinePosition, 0, timelineMax);

	const int timelineParametersOffset = 20;

	//
	// Other timeline parameter controllers
	//

	// Playback speed, and frame skip fields
	ImGui::SetCursorPos(ImVec2(timelineParametersOffset, UI_DEFAULT_TIMELINE_CONTROLS_HEIGHT * 0.4));
	ImGui::PushItemWidth(90);
	ImGui::InputFloat(" Playback speed (ticks/second)", &GuiState.playbackSpeed);
	ImGui::SetCursorPos(ImVec2(timelineParametersOffset, UI_DEFAULT_TIMELINE_CONTROLS_HEIGHT * 0.7));
	ImGui::PushItemWidth(90);
	ImGui::InputInt(" Tick skip amount", &GuiState.tickSkipAmount);

	GuiState.tickSkipAmount = Utils::clamp(GuiState.tickSkipAmount, 0, 100000);

	ImGui::End(); // Timeline Panel

	// Overlayed map image
	static float mapSize = 200.0f; // TODO: make this ui slider?

	GuiState.mapTexture->getWidthHeight(&w, &h);

	float maxMapScaleDimension = max(w, h);

	ImVec2 mapDimensions = ImVec2(mapSize * w / maxMapScaleDimension, mapSize * h / maxMapScaleDimension);

	if (GuiState.doShowMap) {
		// mapDimensions = ImVec2(state.mapTextureDimensions[0], state.mapTextureDimensions[1]);
		ImGui::SetNextWindowSize(mapDimensions + ImVec2(GuiState.padding, 0 /*menuBarHeight*/ + GuiState.padding), 0);
		ImGui::SetNextWindowPos(ImVec2(X, menuBarHeight), 0, ImVec2(1,0));
		ImGui::Begin("Map", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
		ImGui::Image((void*)GuiState.mapTexture->getID(), mapDimensions, ImVec2(0, 1), ImVec2(1, 0));
		ImGui::End(); // Map
	}

	// FPS overlay
	if (GuiState.doShowFPSCounter) {
		ImVec2 fpsOverlaySize(imguiIO->FontGlobalScale * 65, (imguiIO->FontGlobalScale * 17) + 10);
		// Generate fps readout text of the form <### fps\0>
		char fpsReadout[8];
		sprintf(fpsReadout, "%i fps", Utils::clamp((int) GuiState.fps, 0, 999));

		ImGui::SetNextWindowPos(ImVec2(0, Y/2), 0, ImVec2(0, 1));
		ImGui::SetNextWindowSize(fpsOverlaySize);
		ImGui::Begin("FPS", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
		ImGui::Text(fpsReadout);
		ImGui::End(); // FPS
	}	
}

#define SCENE_SELECT_WINDOW_DEFAULT_X 450
#define SCENE_SELECT_WINDOW_DEFAULT_Y 180

std::string runSceneSelectWindow(GLFWwindow* window, const std::string& sceneDirectory, bool& cancelProgram) {
	// Get all scene files in target directory
	std::vector<std::string> files = Utils::getFilesInDirectory(sceneDirectory.c_str());
	std::vector<std::string> scenes;

	std::string selectedScene;

	// Filter files found for files ending in scene
	int nameLength;
	for (auto f : files) {
		if (Utils::hasEnding(f, "scene")) {
			nameLength = f.size();
			scenes.push_back(f.substr(0, nameLength - 6)); // Chop .scene extention from name (will add back in on return)
		}
	}

	int selectedItem = 0;

	// Make a list of c style strings for ImGui to be able to use
	const char** scenes_cstr = new const char* [scenes.size()];

	for (int i = 0; i < scenes.size(); i++){
		*(scenes_cstr + i) = scenes[i].c_str();
	}

	// Cache default size of window so we can set it back at the end
	int originalWindowSize[2];
	glfwGetWindowSize(window, &originalWindowSize[0], &originalWindowSize[1]);

	glfwSetWindowSize(window, SCENE_SELECT_WINDOW_DEFAULT_X, SCENE_SELECT_WINDOW_DEFAULT_Y);

	bool doBreak = false;

	float lastPress = 0.0f;

	while (!doBreak) {
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
			glfwSetWindowShouldClose(window, true);
		}

		if (glfwWindowShouldClose(window)) {
			cancelProgram = true;
			return "";
		}

		// Enter should load the selected scene
		if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS){
			selectedScene = *(scenes_cstr + selectedItem);
			break;
		}	

		// Arrow keys should seek through the list
		// NOTE: This if guard is to make sure the user can't trigger the up and down keys every frame
		if (glfwGetTime() - lastPress > 0.1){
			lastPress = glfwGetTime();

			if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
				selectedItem = Utils::clamp(++selectedItem, 0, (int)scenes.size() - 1);
			}

			if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
				selectedItem = Utils::clamp(--selectedItem, 0, (int)scenes.size() - 1);
			}
		}

		int windowX, windowY;

		glfwGetWindowSize(window, &windowX, &windowY);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::SetNextWindowPos(ImVec2());
		ImGui::SetNextWindowSize(ImVec2(windowX, windowY));
		ImGui::Begin("LoadScene", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

		ImGui::Spacing();
		ImGui::Text("Select Scene To Load");

		ImGui::PushItemWidth(windowX - 15);
		ImGui::ListBox("##Scenes", &selectedItem, scenes_cstr, scenes.size(), 5);

		static ImVec2 buttonDimensions((windowX - 25)/2, 25);

		if (ImGui::Button("Load", buttonDimensions)){
			selectedScene = *(scenes_cstr + selectedItem);
			doBreak = true; // Can't just break immediately because ImGui needs to run the subsequent cleanup functions for this frame
		}
		
		ImGui::SameLine(0, 10);

		if (ImGui::Button("Cancel", buttonDimensions)) {
			glfwSetWindowShouldClose(window, true);
		}

		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	// done

	delete[] scenes_cstr;

	glfwSetWindowSize(window, originalWindowSize[0], originalWindowSize[1]);

	return selectedScene + ".scene";
}
