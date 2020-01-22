#define STB_IMAGE_IMPLEMENTATION

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <Renderer.h>
#include <Material.h>
#include <Utils.h>

#include <vector>
#include <fstream>

#include <chrono>

void windowResizeCallback(GLFWwindow* window, int x, int y) {
	printf("Window resized to %d by %d\n", x, y);
}

int main() {
	// Init Context
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "GLTest", NULL, NULL);

	if (window == NULL) {
		std::cout << "Failed to create window...\n";
		glfwTerminate();
		return -1;
	}

	//auto test = [](GLFWwindow* window, int x, int y) {printf("Window resized to %d by %d\n", x, y); };
	//glfwSetWindowSizeCallback(window, test);

	Renderer* renderer = new Renderer(window);

	// renderer->loadScene("D:/projects/repos/GLTest/scenes/mosport_low.scene");
	renderer->loadScene("C:/user/maxto/OneDrive/Documents/Hacking/RacetrackStats/scenes/testingScene_laptop.scene");

	return 0;

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