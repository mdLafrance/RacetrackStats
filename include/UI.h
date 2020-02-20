#pragma once

#include <vector>
#include <string>

#include <imgui.h>
#include <WorldState.h>
#include <Utils.h>

#define UI_DEFAULT_TIMELINE_CONTROLS_HEIGHT 150

extern _WorldState WorldState;

struct _GuiState {
	// Global states
	bool sceneOpen = false;
	float fontSize;
	int padding = 17;

	unsigned int mapTexture;
	float mapTextureDimensions[2] = { 0,0 };

	double fps; 
	
	// Render states
	float FOV;
	float nearClipPlane;
	float farClipPlane;
	float brightness;

	// Cached Data fields
	bool* dataFieldsEnabled = nullptr; // for some reason, bool vector returns proxy reference in [] operator in stl
	std::vector<std::string> dataFields;
	int numberOfDataTypes;
	int numberOfTimePoints;

	float* TEST;

    // Gui states
    int timelinePosition = 0;
	bool isPlaying = false;
	float playbackSpeed = 1.0f; // Ticks per second
	int tickSkipAmount = 1; // Amount of ticks that should be skipped with arrows

	// Menu bar state
	bool selected_menu_File = false;
	bool selected_menu_File_Open = false;
	bool selected_menu_Options = false;
	bool selected_menu_Options_doShowMap = true;
	bool selected_menu_Options_doShowFPSCounter = true;
};

ImVec2 addImVec2(const ImVec2& a, const ImVec2& b) { // + operator not defined??
	return ImVec2(a[0] + b[0], a[1] + b[1]);
}

void drawUI(_GuiState& state) {
	// Main menu drop down bar
	int menuBarHeight = state.fontSize * 10 + 12; // eyeballing this a bit lol

	int X, Y, halfX, halfY;
	X = WorldState.windowX;
	Y = WorldState.windowY;
	halfX = X / 2;
	halfY = Y / 2;

	// Reset necessary states
	state.selected_menu_File_Open = false;

	if (ImGui::BeginMainMenuBar()) {

		if (ImGui::BeginMenu("File", &state.selected_menu_File)) {
			ImGui::MenuItem("Open", NULL, &state.selected_menu_File_Open);
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Options", &state.selected_menu_Options)) {

			// GUI OPTIONS

			ImGui::Separator();
			ImGui::Separator();

			ImGui::Text("GUI Options");
			ImGui::Separator();
			ImGui::MenuItem("Show Map", NULL, &state.selected_menu_Options_doShowMap);
			ImGui::MenuItem("Show FPS Counter", NULL, &state.selected_menu_Options_doShowFPSCounter);
			ImGui::Text("Font Size");
			ImGui::SameLine(300, 0);
			ImGui::PushItemWidth(100);
			ImGui::InputFloat("##Font size", &state.fontSize, 0, 0, "%.1f");
			
			state.fontSize = state.fontSize > 3 ? 3 : state.fontSize; // things start getting weird above 3 font lol

			ImGui::Separator();
			ImGui::Separator();

			// RENDER OPTIONS
			// TODO: make these work

			ImGui::Text("Render Options");
			ImGui::Separator();

			ImGui::Text("FOV");
			ImGui::SameLine(300, 0);
			ImGui::InputFloat("##FOV", &state.FOV, 0, 0, "%.2f");

			ImGui::Text("Near Clip Plane");
			ImGui::SameLine(300, 0);
			ImGui::InputFloat("##Near Clip Plane", &state.nearClipPlane, 0, 0, "%.2f");

			ImGui::Text("Far Clip Plane");
			ImGui::SameLine(300, 0);
			ImGui::InputFloat("##Far Clip Plane", &state.farClipPlane, 0, 0, "%.2f");

			ImGui::Text("Brightness");
			ImGui::SameLine(300, 0);
			ImGui::SliderFloat("##Brightness", &state.brightness, -1, 1);
			
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
            if (ImGui::Button("All")) {
                memset(state.dataFieldsEnabled, 1, state.dataFields.size());
            }

            ImGui::SameLine(0, 10);

            if (ImGui::Button("None")) {
                memset(state.dataFieldsEnabled, 0, state.dataFields.size());
            }

            // Add all toggles for data types
			for (int i = 0; i < state.dataFields.size(); i++) {
				ImGui::Checkbox(state.dataFields[i].c_str(), state.dataFieldsEnabled + i);
			}
		}
	} // Data type dropdown

    // Main Graph
	if (state.TEST) {
		ImGui::PlotLines("Data", state.TEST, 20, 0, 0, 0, 1000000, ImVec2(X, 50), 1);
	}

	ImGui::End(); // Data Panel

	// Timeline panel
	ImGui::SetNextWindowSize(ImVec2(X, UI_DEFAULT_TIMELINE_CONTROLS_HEIGHT));
	ImGui::SetNextWindowPos(ImVec2(0, Y - UI_DEFAULT_TIMELINE_CONTROLS_HEIGHT), 0, ImVec2(0, 0));
	ImGui::Begin("Timeline Panel", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

	ImGui::PushItemWidth(X - state.padding);
	ImGui::SliderInt("##Timeline", &state.timelinePosition, 0, state.numberOfTimePoints);

	// Params for timeline controllers
	const int playButtonDimensions = 50;
	const int arrowButtonDimensions = 40;
	const int halfArrowButtonDimensions = arrowButtonDimensions / 2;

	const int halfDiff = (playButtonDimensions - arrowButtonDimensions) / 2;

	const int arrowButtonOffset = 60;

	int timeLineButtonLocalVerticalAlign = UI_DEFAULT_TIMELINE_CONTROLS_HEIGHT / 2 - 10;

	ImVec2 pbDimensions = ImVec2(playButtonDimensions, playButtonDimensions); // play button
	ImVec2 bfDimensions = ImVec2(arrowButtonDimensions, arrowButtonDimensions); // back forward

	// Play Button
	// TODO: replace with image
	std::string buttonText = state.isPlaying ? "Pause" : "Play";

	ImGui::SetCursorPos(ImVec2((X - playButtonDimensions)/2, timeLineButtonLocalVerticalAlign));
	if (ImGui::Button(buttonText.c_str(), pbDimensions)) {
		if (state.isPlaying) {
			state.isPlaying = false;
		}
		else {
			state.isPlaying = true;
			state.timelinePosition += 1;
		}
	}

	// Back and forward buttons
	ImGui::SetCursorPos(ImVec2(halfX - halfArrowButtonDimensions - arrowButtonOffset, timeLineButtonLocalVerticalAlign + halfDiff));
	if (ImGui::Button("back", bfDimensions)) state.timelinePosition -= state.tickSkipAmount;

	ImGui::SetCursorPos(ImVec2(halfX - halfArrowButtonDimensions + arrowButtonOffset, timeLineButtonLocalVerticalAlign + halfDiff));
	if (ImGui::Button("forward", bfDimensions)) state.timelinePosition += state.tickSkipAmount;

	state.timelinePosition = Utils::clampInt(state.timelinePosition, 0, state.numberOfTimePoints);

	const int timelineParametersOffset = 20;

	// Playback speed, and frame skip fields
	ImGui::SetCursorPos(ImVec2(timelineParametersOffset, UI_DEFAULT_TIMELINE_CONTROLS_HEIGHT * 0.4));
	ImGui::PushItemWidth(90);
	ImGui::InputFloat(" Playback speed (ticks/second)", &state.playbackSpeed);
	ImGui::SetCursorPos(ImVec2(timelineParametersOffset, UI_DEFAULT_TIMELINE_CONTROLS_HEIGHT * 0.7));
	ImGui::PushItemWidth(90);
	ImGui::InputInt(" Tick skip amount", &state.tickSkipAmount);

	state.tickSkipAmount = Utils::clampInt(state.tickSkipAmount, 0, 10000000); // arbitrarily high

	ImGui::End(); // Timeline Panel

	// Overlayed map image
	if (state.selected_menu_Options_doShowMap) {
		ImVec2 mapDimensions(state.mapTextureDimensions[0], state.mapTextureDimensions[1]); // Add vertical padding for title
		ImGui::SetNextWindowSize(addImVec2(mapDimensions, ImVec2(state.padding, 0 /*menuBarHeight*/ + state.padding)), 0);
		ImGui::SetNextWindowPos(ImVec2(X, menuBarHeight), 0, ImVec2(1,0));
		ImGui::Begin("Map", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
		ImGui::Image((void*)state.mapTexture, mapDimensions);
		ImGui::End(); // Map
	}

	// FPS overlay
	if (state.selected_menu_Options_doShowFPSCounter) {
		ImVec2 fpsOverlaySize(state.fontSize * 65, (state.fontSize * 17) + 10);
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
}