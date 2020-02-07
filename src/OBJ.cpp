#include <chrono>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <OBJ.h>

using namespace std;

inline void seekToEndl(ifstream* target){
    char c;
    while(!target->eof()){
        c = target->get();
        if (c == '\n'){
            return;
        }
    }
}

inline string getWord(ifstream* target){
    string word = "";

    char c = target->get();

    while(!target->eof() && (c != ' ') && (c != '\n')){
        word += c;
        c = target->get();
    }

    return word;
}

namespace OBJ
{
	std::map<std::string, OBJMesh*> load(const std::string& target) {
		ifstream f;

		f.open(target);

		if (!f.is_open()) {
			std::cerr << "Couldn't Open File " << target << " For OBJ Loading" << std::endl;
			return std::map<std::string, OBJMesh*>();
		}

		Utils::FileInfo fi = Utils::getFileInfo(target);

		Utils::StopWatch stopWatch;

		std::string line;

		std::vector<float> positions;
		std::vector<float> normals;
		std::vector<float> texCoords;

		int numOfPositions = 0;
		int numOfNormals = 0;
		int numOfTexCoords = 0;

		std::vector<int> faceIndeces;

		bool collectingFaces = false;

		std::string materialLibrary;
		std::string materialName;

		std::string groupName;
		std::string groupParent;

		OBJMesh* currentObject;

		int currentPositionIndex;
		int currentNormalIndex;
		int currentTexCoordIndex;

		std::map<std::string, OBJMesh*> meshes;

		auto closeObject = [&]() {
			collectingFaces = false;

			int numOfFaces = faceIndeces.size() / 9; // 3 components per 3 verteces per face

			std::string fullName = fi.file + '.' + groupName;

			std::cout << "Writing vertex data for " << groupName << " (" << numOfFaces << " tris, ";

			currentObject = new OBJMesh(fullName, Utils::getFileInfo(materialLibrary).file + '.' + materialName, groupParent, target, numOfFaces, numOfPositions, numOfNormals, numOfTexCoords);

			float* va = currentObject->vertexAttributes;

			for (int i = 0; i < 3 * numOfFaces; i++) {
				// add position triplet for each face
				currentPositionIndex = faceIndeces[(3 * i) + 0];
				*(va + (8 * i) + 0) = positions[(3 * currentPositionIndex) + 0];
				*(va + (8 * i) + 1) = positions[(3 * currentPositionIndex) + 1];
				*(va + (8 * i) + 2) = positions[(3 * currentPositionIndex) + 2];

				// add normals triplet for each face
				currentNormalIndex = faceIndeces[(3 * i) + 2];
				*(va + (8 * i) + 3) = normals[(3 * currentNormalIndex) + 0];
				*(va + (8 * i) + 4) = normals[(3 * currentNormalIndex) + 1];
				*(va + (8 * i) + 5) = normals[(3 * currentNormalIndex) + 2];

				// add tex coordinates pair for each face
				currentTexCoordIndex = faceIndeces[(3 * i) + 1];
				*(va + (8 * i) + 6) = texCoords[(2 * currentTexCoordIndex) + 0];
				*(va + (8 * i) + 7) = texCoords[(2 * currentTexCoordIndex) + 1];
			}

			// Add generated object to map
			meshes[fullName] = currentObject;

			// Cleanup for next mesh

			numOfPositions = 0;
			numOfTexCoords = 0;
			numOfNormals = 0;

			faceIndeces.clear();

			std::cout << stopWatch.lap_s() << ")" << std::endl;
		};

		stopWatch.start();

		while (std::getline(f, line)) {
			if (line == "") {
				continue;
			}

			std::vector<std::string> tokens = Utils::split(line, ' ');

			std::string lineType = tokens[0];

			// Comment lines start with #
			if (lineType == "#" || lineType == "s") {
				continue;
			}

			if (collectingFaces && (lineType != "f")) {
				closeObject();
			}

			// New Material
			if (lineType == "mtllib") {
				std::cout << "File using material library " << tokens[1] << std::endl;
				materialLibrary = tokens[1];
				continue;
			}

			if (lineType == "usemtl") {
				materialName = tokens[1];
				continue;
			}

			// New group
			if (lineType == "g") {
				// Don't care about default group
				if (tokens.back() == "default") {
					continue;
				}

				// Name of object is last in the line
				groupName = tokens.back();
				 
				// Direct parent of object precedes name of object if line contains more words than "g <groupName>"
				int s = tokens.size();
				if (s > 2) {
					groupParent = tokens[s-3]; // tokens[-2]
				} 
			}

			// Vertices
			if (lineType == "v") {
				++numOfPositions;
				positions.push_back(std::atof(tokens[1].c_str()));
				positions.push_back(std::atof(tokens[2].c_str()));
				positions.push_back(std::atof(tokens[3].c_str()));

				//std::cout << "(" << ((positions.getSize())/3) << ") "<<"Added point: " << std::atof(tokens[1].c_str()) << " " << std::atof(tokens[2].c_str()) << " " << std::atof(tokens[3].c_str()) << std::endl;
			}

			// Texture Coords
			if (lineType == "vt") {
				++numOfTexCoords;
				texCoords.push_back(std::atof(tokens[1].c_str()));
				texCoords.push_back(std::atof(tokens[2].c_str()));
			}

			// Vertex Normals 
			if (lineType == "vn") {
				++numOfNormals;
				normals.push_back(std::atof(tokens[1].c_str()));
				normals.push_back(std::atof(tokens[2].c_str()));
				normals.push_back(std::atof(tokens[3].c_str()));
			}

			// Faces
			if (lineType == "f") {
				collectingFaces = true;

				// Add each of 1/2/3 4/5/6 7/8/9
				for (int i = 1; i < 4; i++) {
					for (std::string f : Utils::split(tokens[i], '/')) {
						faceIndeces.push_back(std::stoi(f) - 1); // OBJ face indeces are 1-indexed
					}
				}
			}
		}

		closeObject();

		f.close();
		
		for (auto p : meshes) {
			OBJMesh* m = p.second;
			std::cout << "Generating VBO for " << m->getMeshName() << std::endl;
			m->generateBuffers();
		}

		std::cout << "Finished loading file " << target << " (" << stopWatch.total_s() << ")" << std::endl;

		return meshes;
	}
}

std::string OBJMesh::getMeshName() {
	return this->meshName;
}

std::string OBJMesh::getDefaultMaterialName() {
	return this->defaultMaterialName;
}

std::string OBJMesh::getDefaultParentName(){
	return this->defaultParent;
}

unsigned int OBJMesh::getNumberOfFaces() {
	return this->numberOfFaces;
}

void OBJMesh::generateBuffers() {
	// Generate and bind VAO for this mesh
	glGenVertexArrays(1, &this->VAO);
	glBindVertexArray(this->VAO);

	// Generate vertex attribute buffer
	int lenOfVBO = sizeof(float) * 3 * 8 * this->numberOfFaces;
	glGenBuffers(1, &this->VBO);
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	glBufferData(GL_ARRAY_BUFFER, lenOfVBO, this->vertexAttributes, GL_STATIC_DRAW);

	int vertexSize = 8 * sizeof(float);
	// Enable pointer for vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertexSize, (void*)0);

	// Enable pointer for vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertexSize, (void*) (3 * sizeof(float)));

	// Enable pointer for vertex texture coordinates
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, vertexSize, (void*)(6 * sizeof(float)));

	// Clear binds
	glBindVertexArray(0); 
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void OBJMesh::bind() {
	glBindVertexArray(this->VAO);
}

int OBJMesh::getApproxBytes() {
	return (sizeof(float) * (3 * (numOfPositions + numOfNormals)) + (2 * numOfTexCoords)) + (sizeof(OBJ::FaceElements) * 3 * this->numberOfFaces);
}

void OBJMesh::draw() {
	glBindVertexArray(this->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	
	glDrawArrays(GL_TRIANGLES, 0, 3 * this->numberOfFaces);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

OBJMesh::OBJMesh(const std::string& meshName, const std::string& materialName, const std::string& parent, const std::string& origin, const int& numberOfFaces, const int& numberOfPositions, const int& numberOfNormals, const int& numberOfTexCoords) {
	this->meshName = meshName;
	this->defaultMaterialName = materialName;
	this->defaultParent = parent;
	this->origin = origin;

	this->numberOfFaces = numberOfFaces;

	this->numOfPositions = numberOfNormals;
	this->numOfNormals   = numberOfNormals;
	this->numOfTexCoords = numberOfTexCoords;

	this->vertexAttributes = new float[8 * 3 * this->numberOfFaces];
}

OBJMesh::~OBJMesh() {
	delete[] this->vertexAttributes;

	glDeleteBuffers(1, &this->VBO);
	glDeleteVertexArrays(1, &this->VAO);
}
