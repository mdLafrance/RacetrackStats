#pragma once

#include <iostream>
#include <utility>
#include <string.h>
#include <map>
#include <future>
#include <thread>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <Material.h> // Includes texture and shader .h
#include <OBJ.h>
#include <Camera.h>
#include <Object.h>
#include <Light.h>
#include <WorldState.h>

#include <TSQueue.hpp>

#define MAX_LIGHT_COUNT 8

// Global world state instantiated in main.cpp
extern _WorldState WorldState;

// Convenince function
std::string vec3ToString(const glm::vec3& v);

struct Scene {
	std::string name;
	std::string path;
	std::vector<std::string> files;
};

class Renderer {
	GLFWwindow* window;

	GLuint renderTarget;

	Camera* mainCamera;

	std::map<std::string, Texture*> textures;
	std::map<std::string, Shader*> shaders;
	std::map<std::string, OBJMesh*> meshes;
	std::map<std::string, Material*> materials;
	std::map<std::string, Camera*> cameras;

	Scene scene;

	int numOfLights;
	Light lights[MAX_LIGHT_COUNT];
	glm::mat3 lightMatrices[MAX_LIGHT_COUNT];

	long int frameCount;

	float lineWidth;
	float lineWidthMax = 1;

	void resetData();
	void deleteObjects();

public:
	float progress = 0.0f; // Used to store loading progress for various operations so that it can be visible to other threads running loading bars
	bool loading = false; // If the renderer is busy loading, is set to true

	std::map<std::string, Object*> objects;

	void loadScene(const std::string& target);
	void loadMaterialLibrary(const std::string& target);
	void loadOBJ(const std::string& target);

	void registerTexture(const std::string& id, Texture* texture);
	void registerShader(const std::string& id, Shader* shader);
	void registerMesh(const std::string& id, OBJMesh* mesh);
	void registerMaterial(const std::string& id, Material* material);
	void registerCamera(const std::string& id, Camera* camera);

	Object* newObject(const std::string& name);
	Object* getObject(const std::string& name);

	Camera* getMainCamera();
	void setMainCamera(const std::string& id);

	// Draw a line of <color> from <origin> to <end> (in world space).
	// drawOver=true will cause the line to draw on top of all other scene elements.
	void drawLine(const glm::vec3& origin, const glm::vec3& end, const glm::vec3& color, bool drawOver=true);

	void setLineWidth(const float& w);

	// Render the next frame
	void tick(const double& dTime);
	
	Renderer(GLFWwindow* window);
	~Renderer();
};
