#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include <iostream>

#include <imgui.h>

#include <Texture.h>
#include <WorldState.h>
#include <Utils.h>

#define UI_DEFAULT_TIMELINE_CONTROLS_HEIGHT 150

extern _WorldState WorldState;

extern ImGuiIO* imguiIO; // Set in main, handle to the io structure set by imgui (collects mouse clicks etc.)

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
	
	// Render states
	float FOV = 0.0f;
	float nearClipPlane = 0.0f;
	float farClipPlane = 100.0f;
	float brightness = 0.0f;

	bool lineWidthChanged = false;
	float lineWidth = 1;
	float glLineWidthRange[2];

	// Cached Data fields
	bool* dataFieldsEnabled = nullptr; // for some reason, bool vector returns proxy reference in [] operator in stl
	std::vector<std::string> dataFields;
	int numberOfDataTypes = 0;
	int numberOfTimePoints = 0;

    // Gui states
    int timelinePosition = 0;
	bool isPlaying = false;
	float playbackSpeed = 1.0f; // Ticks per second
	int tickSkipAmount = 1; // Amount of ticks that should be skipped with arrows

	// Menu bar state
	bool selected_menu_File = false;
	bool selected_menu_File_Open = false;
	bool selected_menu_Options = false;
	bool doShowMap = true;
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

void setGuiOptionsToDefault(_GuiState& state) {
	state.doShowFPSCounter = true;
	state.doShowMap = true;

	state.cameraSettingsChanged = true;

	state.nearClipPlane = 0.5f;
	state.farClipPlane = 1000.0f;
	state.brightness = 0.0f;
	state.FOV = 0.0f;

	state.lineWidth = 1.0f;
	state.lineWidthChanged = true;
}

std::string imVec2ToString(const ImVec2& v) { // ostream<< operator not defined??
	char s[32];
	sprintf(s, "[%.3f, %.3f]", v.x, v.y);
	return std::string(s);
}

ImVec2 addImVec2(const ImVec2& a, const ImVec2& b) { // + operator not defined??
	return ImVec2(a[0] + b[0], a[1] + b[1]);
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
	*/
	assert(textures != nullptr);

	enum ButtonState { Normal = 0, Hovered = 1, Clicked = 2 };

	ImVec2 mousePos = ImGui::GetMousePos();

	ImVec2 boundsX(cursorPos[0], cursorPos[0] + buttonSize[0]);
	ImVec2 boundsY(cursorPos[1], cursorPos[1] + buttonSize[1]);

	ButtonState bs;

	// std::cout << imVec2ToString(boundsX) << " : " << imVec2ToString(boundsY) << " <- " << imVec2ToString(mousePos) << std::endl; // For debugging, reads out the bounds and mouse pos

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

void drawUI(_GuiState& state) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Main menu drop down bar
	int menuBarHeight = imguiIO->FontGlobalScale * 10 + 12; // eyeballing this a bit lol

	int X, Y, halfX, halfY;
	X = WorldState.windowX;
	Y = WorldState.windowY;
	halfX = X / 2;
	halfY = Y / 2;

	int w, h; // Used to hold various button and image widths and heights later on

	// NOTE: Spent time looking for the right style element color for this, but ended up just eyedropping the color
	ImVec4 bgColor = ImVec4(0.056, 0.056, 0.056, 1.0f);// ImGui::GetStyleColorVec4(ImGuiCol_TitleBg); // Cache background color for use with the icon buttons later

	// Reset necessary states
	state.selected_menu_File_Open = false;

	if (ImGui::BeginMainMenuBar()) {

		if (ImGui::BeginMenu("File", &state.selected_menu_File)) {
			ImGui::MenuItem("Open", NULL, &state.selected_menu_File_Open);
			ImGui::EndMenu();
			ImGui::Separator();
			ImGui::Separator();
		}

		if (ImGui::BeginMenu("Options", &state.selected_menu_Options)) {

			// GUI OPTIONS

			
			ImGui::Separator();
			ImGui::Separator();

			ImGui::Text("GUI Options");
			ImGui::Separator();
			ImGui::MenuItem("Show Map", NULL, &state.doShowMap);
			ImGui::MenuItem("Show FPS Counter", NULL, &state.doShowFPSCounter);
			ImGui::Text("Font Size");
			ImGui::SameLine(300, 0);
			ImGui::PushItemWidth(100);
			ImGui::InputFloat("##Font size", &imguiIO->FontGlobalScale, 0, 0, "%.1f");

			imguiIO->FontGlobalScale = imguiIO->FontGlobalScale > 3 ? 3 : imguiIO->FontGlobalScale; // things start getting weird above 3 font lol

			ImGui::Separator();
			ImGui::Separator();

			// RENDER OPTIONS

			ImGui::Text("Render Options");
			ImGui::Separator();

			const float FOVbounds = 1.0f;
			ImGui::Text("FOV -/+");
			ImGui::SameLine(300, 0);
			if (ImGui::SliderFloat("##FOV", &state.FOV, -FOVbounds, FOVbounds)) state.cameraSettingsChanged = true;

			// ImGui::Text("Near Clip Plane");
			// ImGui::SameLine(300, 0);
			// if (ImGui::InputFloat("##Near Clip Plane", &state.nearClipPlane, 0, 0, "%.2f")) state.cameraSettingsChanged = true;

			ImGui::Text("Far Clip Plane");
			ImGui::SameLine(300, 0);
			if (ImGui::InputFloat("##Far Clip Plane", &state.farClipPlane, 0, 0, "%.2f")) state.cameraSettingsChanged = true;

			ImGui::Text("Brightness +/-");
			ImGui::SameLine(300, 0);
			ImGui::SliderFloat("##Brightness", &state.brightness, -1, 1);

			// gl may not enable line width changes, in which case the max is 1
			if (state.glLineWidthRange[1] > 1) {
				ImGui::Text("Line Width");
				ImGui::SameLine(300, 0);
				if (ImGui::SliderFloat("##Line Widdth", &state.lineWidth, state.glLineWidthRange[0], Utils::clamp(state.glLineWidthRange[1], 1.0f, 10.0f))) state.lineWidthChanged = true;
			}

			ImGui::Separator();
			ImGui::Separator();

			ImGui::PushItemWidth(300);
			if (ImGui::Button("Reset to Defaults")) setGuiOptionsToDefault(state);

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	} // Main menu drop down bar

	// Data panel
	ImGui::SetNextWindowSize(ImVec2(X, halfY - UI_DEFAULT_TIMELINE_CONTROLS_HEIGHT));
	ImGui::SetNextWindowPos(ImVec2(0, halfY), 0, ImVec2(0, 0));
	ImGui::Begin("Data", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

	// Dropdown to enable or disable data type display
	if (state.sceneOpen) {
		if (ImGui::CollapsingHeader("Display Data Types")) {
			// 'All' and 'None' buttons

			// if (ImGui::Button("All")) {
			//     memset(state.dataFieldsEnabled, 1, state.dataFields.size());
			// }

			// ImGui::SameLine(0, 10);

			if (ImGui::Button("None")) {
				memset(state.dataFieldsEnabled, 0, state.dataFields.size());
			}

			// Add all toggles for data types
			for (int i = 0; i < state.dataFields.size(); i++) {
				ImGui::Checkbox(state.dataFields[i].c_str(), state.dataFieldsEnabled + i);
				if (*(state.dataFieldsEnabled + i)) {
				}
			}
		}
	} // Data type dropdown

	ImGui::End(); // Data Panel

	// Timeline panel
	ImVec2 timelinePanelPosition(0, Y - UI_DEFAULT_TIMELINE_CONTROLS_HEIGHT);

	ImGui::SetNextWindowSize(ImVec2(X, UI_DEFAULT_TIMELINE_CONTROLS_HEIGHT));
	ImGui::SetNextWindowPos(timelinePanelPosition, 0, ImVec2(0, 0));
	ImGui::Begin("Timeline Panel", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

	ImGui::PushItemWidth(X - state.padding);
	ImGui::SliderInt("##Timeline", &state.timelinePosition, 0, state.numberOfTimePoints);

	//
	// Timeline control buttons
	//

	int buttonState = 0; // 0 is normal, 1 is hovered, 2 is pressed and hovered. 
	Texture* buttonTexture;

	// Determine the right color for the button

	// Params for timeline controllers
	bool mouseButtonIsPressed = imguiIO->MouseDown[0]; // Continuous signal
	bool mouseButtonPressedThisFrame = ImGui::IsMouseClicked(0, false); // Only registers on first frame pressed

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
 
	Texture** ppButton = state.isPlaying ? state.pauseButtonTextures : state.playButtonTextures; //pause/play button, which texture set should be used?

	// Cursor is where ImGui will draw the item
	cursorPos = {(float) (X - playButtonDimensions) / 2, (float)timeLineButtonLocalVerticalAlign };
	ImGui::SetCursorPos(cursorPos);

	// NOTE: need to add the position of the parent panel to convert into 'global' app pixel coordinates, since cursor is in 'local' panel window coordinates
	if (ImageButton2(ppButton, addImVec2(cursorPos, timelinePanelPosition), pbDimensions, false)) {
		// Flip the button to other state
		if (state.isPlaying) {
			state.isPlaying = false;
		}
		else {
			state.isPlaying = true;
			state.timelinePosition += Utils::sign(state.playbackSpeed); // Increment the timeline on the frame when the play button is pressed
		}
	}

	//
	// Back and forward buttons
	//

	// Back Button
	cursorPos = {(float) halfX - halfArrowButtonDimensions - arrowButtonOffset, (float) timeLineButtonLocalVerticalAlign + halfDiff };
	ImGui::SetCursorPos(cursorPos);

	if (ImageButton2(state.skipButtonTextures, addImVec2(cursorPos, timelinePanelPosition), bfDimensions, true)) state.timelinePosition -= state.tickSkipAmount;

	// // Forward button
	cursorPos = {(float) halfX - halfArrowButtonDimensions + arrowButtonOffset, (float) timeLineButtonLocalVerticalAlign + halfDiff };
	ImGui::SetCursorPos(cursorPos);

	if (ImageButton2(state.skipButtonTextures, addImVec2(cursorPos, timelinePanelPosition), bfDimensions)) state.timelinePosition += state.tickSkipAmount;

	// Clamp timeline within bounds
	state.timelinePosition = Utils::clamp(state.timelinePosition, 0, state.numberOfTimePoints);

	const int timelineParametersOffset = 20;

	//
	// Other timeline parameter controllers
	//

	// Playback speed, and frame skip fields
	ImGui::SetCursorPos(ImVec2(timelineParametersOffset, UI_DEFAULT_TIMELINE_CONTROLS_HEIGHT * 0.4));
	ImGui::PushItemWidth(90);
	ImGui::InputFloat(" Playback speed (ticks/second)", &state.playbackSpeed);
	ImGui::SetCursorPos(ImVec2(timelineParametersOffset, UI_DEFAULT_TIMELINE_CONTROLS_HEIGHT * 0.7));
	ImGui::PushItemWidth(90);
	ImGui::InputInt(" Tick skip amount", &state.tickSkipAmount);

	state.tickSkipAmount = Utils::clamp(state.tickSkipAmount, 0, 10000000); // Mathf.infinity?

	ImGui::End(); // Timeline Panel

	// Overlayed map image
	static float mapSize = 200.0f; // TODO: make this ui slider?

	state.mapTexture->getWidthHeight(&w, &h);

	float maxMapScaleDimension = max(w, h);

	ImVec2 mapDimensions = ImVec2(mapSize * w / maxMapScaleDimension, mapSize * h / maxMapScaleDimension);

	if (state.doShowMap) {
		// mapDimensions = ImVec2(state.mapTextureDimensions[0], state.mapTextureDimensions[1]);
		ImGui::SetNextWindowSize(addImVec2(mapDimensions, ImVec2(state.padding, 0 /*menuBarHeight*/ + state.padding)), 0);
		ImGui::SetNextWindowPos(ImVec2(X, menuBarHeight), 0, ImVec2(1,0));
		ImGui::Begin("Map", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
		ImGui::Image((void*)state.mapTexture->getID(), mapDimensions);
		ImGui::End(); // Map
	}

	// FPS overlay
	if (state.doShowFPSCounter) {
		ImVec2 fpsOverlaySize(imguiIO->FontGlobalScale * 65, (imguiIO->FontGlobalScale * 17) + 10);
		// Generate fps readout text of the form <### fps\0>
		char fpsReadout[8];
		int fpsInt = (int)state.fps;
		int fpsClipped = fpsInt >= 1000 ? 999 : fpsInt;
		sprintf(fpsReadout, "%i fps", fpsClipped);

		ImGui::SetNextWindowPos(ImVec2(0, Y/2), 0, ImVec2(0, 1));
		ImGui::SetNextWindowSize(fpsOverlaySize);
		ImGui::Begin("FPS", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
		ImGui::Text(fpsReadout);
		ImGui::End(); // FPS
	}	
	
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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