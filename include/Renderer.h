#pragma once

#include <iostream>
#include <utility>
#include <string.h>
#include <map>
#include <future>
#include <thread>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <Material.h>
#include <Texture.h>
#include <Shader.h>
#include <OBJ.h>
#include <Camera.h>
#include <Object.h>
#include <Light.h>
#include <WorldState.h>
#include <Skybox.h>

#define MAX_LIGHT_COUNT 8

// Global world state instantiated in main.cpp
extern _WorldState WorldState;

struct Scene {
	std::string name;
	std::string path;
	std::vector<std::string> files;
};

struct LineData {
	glm::vec3 origin;
	glm::vec3 end;
	glm::vec3 color;
	bool drawOver;
};

class Renderer {
	GLFWwindow* window;

	GLuint renderTarget;

	Camera* mainCamera;

	Skybox* skybox;

	Scene scene;

	int numOfLights;
	Light lights[MAX_LIGHT_COUNT];
	glm::mat3 lightMatrices[MAX_LIGHT_COUNT];

	long int frameCount;

	void resetData();
	void deleteObjects();

	void drawLine(const glm::vec3& origin, const glm::vec3& end, const glm::vec3& color, bool drawOver=true);

	float lineWidth;
	float lineWidthMax = 1;
	std::vector<LineData> linesToDraw;

public:
	std::map<std::string, Texture*> textures;
	std::map<std::string, Shader*> shaders;
	std::map<std::string, OBJMesh*> meshes;
	std::map<std::string, Material*> materials;
	std::map<std::string, Camera*> cameras;
	std::map<std::string, Skybox*> skyboxes;

	float progress = 0.0f; // Used to store loading progress for various operations so that it can be visible to other threads running loading bars
	bool loading = false; // If the renderer is busy loading, is set to true

	std::vector<Object*> objects;

	void loadScene(const std::string& target);

	void registerTexture(const std::string& id, Texture* texture);
	void registerShader(const std::string& id, Shader* shader);
	void registerMesh(const std::string& id, OBJMesh* mesh);
	void registerMaterial(const std::string& id, Material* material);
	void registerCamera(const std::string& id, Camera* camera);
	void registerSkybox(const std::string& id, Skybox* skybox);

	Object* newObject(const std::string& name);
	Object* getObject(const std::string& name);

	Camera* getMainCamera();
	void setMainCamera(const std::string& id);

	void setSkybox(const std::string& name);

	// Draw a line of <color> from <origin> to <end> (in world space).
	// drawOver=true will cause the line to draw on top of all other scene elements.
	// NOTE: Drawing lines outside of render loop causes strange behavior, so instead, oustide of renderer can use addLine to queue lines to be drawn during next render iteration
	void addLine(const glm::vec3& origin, const glm::vec3& end, const glm::vec3& color, bool drawOver=true);

	void setLineWidth(const float& w);

	// Render the next frame
	void tick(const double& dTime);
	
	Renderer(GLFWwindow* window);
	~Renderer();
};
