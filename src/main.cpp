#define STB_IMAGE_IMPLEMENTATION

#define WINDOW_TITLE "Racetrack Stats"

// Name of project this is for
// Might want to extract some functionality for later use, so some functionality/dependency will be guarded by this
#define __RACETRACK_STATS__

#include <cstdlib>

#include <vector>
#include <fstream>
#include <chrono>
#include <iostream>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <Renderer.h>
#include <Material.h>
#include <Utils.h>
#include <WorldState.h>
#include <Light.h>

_WorldState WorldState = { 
	WINDOW_DEFAULT_X, // Starting window width
	WINDOW_DEFAULT_Y, // Starting window height
	{0.0f, 0.0f, 0.0f},  // vec3 ambient color for the scene
	nullptr			  // (string) path to root of executable (set in main)
};

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
		printf("Window resized to %d by %d\n", x, y); 
		::WorldState.windowX = x;
		::WorldState.windowY = y;
	};

	glfwSetWindowSizeCallback(window, windowResizeCallbackFunction);
	glfwSetErrorCallback(Utils::glfwErrorCallbackFunction);

	bool onMSI = std::getenv("MSI") != nullptr;

	if (onMSI) {
		::WorldState.projectRoot = "D:/Hacking/RacetrackStats";
	}
	else {
		::WorldState.projectRoot = "C:/Users/maxto/OneDrive/Documents/Hacking/RacetrackStats";
	}

	Renderer* renderer = new Renderer(window);

	if (onMSI) {
		renderer->loadScene(std::string(::WorldState.projectRoot) + "/resources/scenes/testingScene.scene");
	}
	else { 
		renderer->loadScene(std::string(::WorldState.projectRoot) + "/resources/scenes/testingScene_laptop.scene");
	}

	std::chrono::time_point<std::chrono::steady_clock> t1, t2;

	double dTime = 0;
	
	while (!glfwWindowShouldClose(window)) {
		t1 = std::chrono::steady_clock::now();

		renderer->tick(dTime);

		t2 = std::chrono::steady_clock::now();

		dTime = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1000000.0f; // Microsecond conversion into fraction of second
	}

	return 0;
}