#pragma once

#define WINDOW_DEFAULT_X 800
#define WINDOW_DEFAULT_Y 600

#include <iostream>
#include <utility>
#include <string.h>
#include <map>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <Material.h> // Includes texture and shader .h
#include <OBJ.h>
#include <Camera.h>
#include <Object.h>
#include <WorldState.h>

// Global world state instantiated in main.cpp
extern _WorldState WorldState;

struct Scene {
	std::string name;
	std::string path;
	std::vector<std::string> files;
};

class Renderer {
	GLFWwindow* window;

	Camera* mainCamera;

	std::map<std::string, Texture*> textures;
	std::map<std::string, Shader*> shaders;
	std::map<std::string, OBJMesh*> meshes;
	std::map<std::string, Material*> materials;
	std::map<std::string, Camera*> cameras;

	std::map<std::string, Object*> objects;

	Scene scene;

	unsigned int line_VAO;

public:
	void loadScene(const std::string& target);
	void loadMaterialLibrary(const std::string& target);
	void loadOBJ(const std::string& target);

	void registerTexture(const std::string& id, Texture* texture);
	void registerShader(const std::string& id, Shader* shader);
	void registerMesh(const std::string& id, OBJMesh* mesh);
	void registerMaterial(const std::string& id, Material* material);
	void registerCamera(const std::string& id, Camera* camera);

	Object* newObject(const std::string& name);

	Camera* getMainCamera();
	void setMainCamera(const std::string& id);

	void drawLine(const glm::vec3& origin, const glm::vec3& end, const glm::vec4& color);

	void deleteObjects();

	void tick(const double& dTime);
	
	Renderer(GLFWwindow* window);
	~Renderer() {};
};
