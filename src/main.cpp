#define STB_IMAGE_IMPLEMENTATION

#define WINDOW_DEFAULT_X 800
#define WINDOW_DEFAULT_Y 600
#define WINDOW_TITLE "Racetrack Stats"

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

_WorldState WorldState = { WINDOW_DEFAULT_X, WINDOW_DEFAULT_Y };

int main() {
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

	Renderer* renderer = new Renderer(window);

	std::string projectRoot;

	if (std::getenv("MSI") != nullptr) { // Env variable defined on main workstation
		renderer->loadScene("D:/Hacking/RacetrackStats/scenes/testingScene.scene");
	}
	else { // My other machine lol
		renderer->loadScene("C:/Users/maxto/OneDrive/Documents/Hacking/RacetrackStats/scenes/testingScene_laptop.scene");
	}

	std::chrono::time_point<std::chrono::steady_clock> t1, t2;

	double dTime = 0;

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	while (!glfwWindowShouldClose(window)) {
		t1 = std::chrono::steady_clock::now();

		renderer->tick(dTime);

		t2 = std::chrono::steady_clock::now();

		dTime = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1000000.0f; // Microsecond conversion into fraction of second
	}

	return 0;
}