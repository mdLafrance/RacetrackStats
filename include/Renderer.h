#pragma once

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

struct _WorldState {
	int windowX;
	int windowY;
};

// Global world state instantiated in main.cpp
extern _WorldState WorldState;

struct Scene {
	std::string name;
	std::string path;
	std::vector<std::string> files;
};

class Renderer {
	int windowX;
	int windowY;

	GLFWwindow* window;

	Camera* mainCamera;

	std::map<std::string, Texture*> textures;
	std::map<std::string, Shader*> shaders;
	std::map<std::string, OBJMesh*> meshes;
	std::map<std::string, Material*> materials;
	std::map<std::string, Camera*> cameras;

	std::map<std::string, Object*> objects;

	Scene scene;

public:
	void loadScene(const std::string& target);
	void loadMaterialLibrary(const std::string& target);
	void loadOBJ(const std::string& target);

	inline void getWindowDimensions(int* width, int* height);
	inline void updateWindowDimensions(int width, int height);
	inline void updateWindowDimensions(GLFWwindow* window, int width, int height);

	void registerTexture(const std::string& id, Texture* texture);
	void registerShader(const std::string& id, Shader* shader);
	void registerMesh(const std::string& id, OBJMesh* mesh);
	void registerMaterial(const std::string& id, Material* material);
	void registerCamera(const std::string& id, Camera* camera);

	Object* newObject(const std::string& name);

	void setMainCamera(const std::string& id);

	void start();

	void deleteObjects();

	void tick(const double& dTime);
	
	Renderer(GLFWwindow* window);
	~Renderer();
};
