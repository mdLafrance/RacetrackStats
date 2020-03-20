#pragma once

#include <iostream>
#include <stdio.h>
#include <string>
#include <string.h>
#include <fstream>
#include <map>
#include <chrono>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <Utils.h>

class OBJMesh {
	std::string meshName; // Name of the mesh
	std::string defaultMaterialName; // Name of the default material to be associated with the mesh
	std::string defaultParent; // Name of parent specified for mesh
	std::string origin; // Path to origin file of the mesh

	int numberOfFaces;

	float* vertexData;

	unsigned int VBO;
	unsigned int VAO;
	unsigned int EBO;

	bool loaded = false;

public:
	bool isLoaded();

	std::string getMeshName();
	std::string getDefaultMaterialName();
	std::string getDefaultParentName();
	unsigned int getNumberOfFaces();

	void generateBuffers();

	void bind();

	void draw();

	OBJMesh(const std::string& meshName, const std::string& materialName, const std::string& parent, const std::string& origin, const int& numberOfTriangles, float* vertexData);
	~OBJMesh();
};

namespace OBJ
{
	std::vector<OBJMesh*> load(const std::string& target);
}