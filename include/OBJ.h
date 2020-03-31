#pragma once

#include <assert.h>

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

class OBJMesh;

namespace OBJ
{
	struct FaceMaterials {
		std::string material;

		int range[2] = { 0, 0 };

		FaceMaterials(const std::string& material, const int& start, const int& end) {
			this->material = material;
			this->range[0] = start;
			this->range[1] = end;
		}

	};

	std::vector<OBJMesh*> load(const std::string& target);
}

class OBJMesh {
	std::string meshName; // Name of the mesh
	std::string defaultMaterialName; // Name of the default material to be associated with the mesh
	std::string defaultParent; // Name of parent specified for mesh
	std::string origin; // Path to origin file of the mesh

	int numberOfFaces;

	float* vertexData;

	unsigned int VBO;
	unsigned int VAO;

	bool multipleMaterials = false;
	std::vector<OBJ::FaceMaterials> _faceMaterials;

	bool loaded;

public:
	bool isLoaded();

	bool isUsingMultipleMaterials();
	std::vector<OBJ::FaceMaterials> getFaceMaterials();

	std::string getMeshName();
	std::string getDefaultMaterialName();
	std::string getDefaultParentName();
	unsigned int getNumberOfFaces();

	void generateBuffers();

	void bind();
	void unbind();

	void draw(int start = -1, int end = -1);

	OBJMesh(
		const std::string& meshName, 
		const std::string& parent, 
		const std::string& origin, 

		std::vector<OBJ::FaceMaterials> faceMaterials,
		const int& numberOfTriangles, 
		float* vertexData);

	~OBJMesh();
};