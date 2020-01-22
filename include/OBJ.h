#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <map>

#include <Utils.h>

class OBJMesh;

namespace OBJ
{
	typedef struct {
		float position[3];
		float normal[3];
		float texCoords[2];
	} Vertex;

	typedef struct {
		float x;
		float y;
		float z;
	} R3;

	typedef struct {
		float u;
		float v;
	} TexPoint;

	typedef struct {
		int posID;
		int normID;
		int texID;
	} FaceElements;

	struct OBJData {
		std::string name;
		std::string mtllib;
		std::vector<OBJMesh*> meshes;
	};

	std::map<std::string, OBJMesh*> load(const std::string& target);
	//OBJData load(const std::string& target);
}

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

	void generateBuffers();

	int getApproxBytes();

	void draw();

	OBJMesh(const std::string& meshName, const std::string& materialName, const std::string& parent, const std::string& origin, const int& numberOfFaces, const int& numberOfPositions, const int& numberOfNormals, const int& numberOfTexCoords);
	~OBJMesh();
};