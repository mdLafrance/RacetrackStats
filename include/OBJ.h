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

	std::vector<OBJ::FaceMaterials> faceMaterials;

	bool loaded = false;

public:
	bool isLoaded();

	const std::vector<OBJ::FaceMaterials>& getFaceMaterials();

	std::string getMeshName();
	std::string getDefaultMaterialName();
	std::string getDefaultParentName();
	unsigned int getNumberOfFaces();

	void generateBuffers();

	inline void bind() { glBindVertexArray(this->VAO); };

	inline void unbind() {
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void draw();
	void drawRange(int start = -1, int count = -1);

	OBJMesh(
		const std::string& meshName,
		const std::string& parent,
		const std::string& origin,
		
		const int& numberOfTriangles,
		float* vertexData,
		const std::vector<OBJ::FaceMaterials>& faceMaterials
	);

	~OBJMesh();
};