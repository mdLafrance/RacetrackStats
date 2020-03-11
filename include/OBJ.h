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

// TODO: workflow for objs changed, this class needs to be cleaned up

class OBJMesh {
	std::string meshName; // Name of the mesh
	std::string defaultMaterialName; // Name of the default material to be associated with the mesh
	std::string defaultParent; // Name of parent specified for mesh
	std::string origin; // Path to origin file of the mesh

public:
	int numberOfFaces;
	unsigned int* faces;

	int numOfPositions;
	int numOfNormals;
	int numOfTexCoords;

	float* vertexAttributes;

	bool isLoaded;

	unsigned int VBO;
	unsigned int VAO;
	unsigned int EBO;

	std::string getMeshName();
	std::string getDefaultMaterialName();
	std::string getDefaultParentName();

	unsigned int getNumberOfFaces();

	void bind();

	void generateBuffers(float* data);

	int getApproxBytes();

	void draw();

	OBJMesh(const std::string& meshName, const std::string& materialName, const std::string& parent, const std::string& origin, const int& numberOfFaces, const int& numberOfPositions, const int& numberOfNormals, const int& numberOfTexCoords);
	~OBJMesh();
};

namespace OBJ
{
	std::map<std::string, OBJMesh*> load(const std::string& target);
}