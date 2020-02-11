#include <OBJ.h>

inline void getFirstWord(const char* s, char* dest){
	int i = 0;

	while (s[i] != ' ' && s[i] != '\0'){
		*(dest + i) = s[i];
		i += 1;
	}

	*(dest+i) = '\0';
}

namespace OBJ
{
	std::map<std::string, OBJMesh*> load(const std::string& target) {
		FILE* f;

		f = fopen(target.c_str(), "r");

		if (!f) {
			std::cerr << "Couldn't Open File " << target << " For OBJ Loading" << std::endl;
			return std::map<std::string, OBJMesh*>();
		}

		Utils::StopWatch writeVertex, readData, total;

		total.start();

		Utils::FileInfo fi = Utils::getFileInfo(target);

		const char* line_cstr;
		char firstWord[128];
		char line[1024];
		//std::string line;

		float a, b, c; // Float values read from a line

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
			std::cout << " (" << readData.lap_s() << ")" << std::endl;

			writeVertex.start();

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

			std::cout << writeVertex.lap_s() << ")" << std::endl;
		};

		long lines = 0;

		while (fscanf(f, "%s", firstWord) != EOF) {
			
			++lines;

			//std::cout << '\r' << lines << " [" << p_start << ", " << p_end << "] : " << line << '\r';
			//std::cout << '\r' << lines << " : " << line << '\r';

			if (firstWord[0] == '\n' || firstWord[0] == '#' || firstWord[0] == 's') {
				goto nextline;
			}

			//getFirstWord(line, firstWord);

			if (collectingFaces && !(firstWord[0] == 'f')) {
				closeObject();
			}

			// New Material
			if (strcmp(firstWord, "mtllib") == 0) {
				fscanf(f, "%s", line); 
				materialLibrary = std::string(line);
				std::cout << "File using material library " << materialLibrary << std::endl;
				goto nextline;
			}

			if (strcmp(firstWord, "usemtl") == 0) {
				fscanf(f, "%s", line); 
				materialName = std::string(line);
				goto nextline;
			}

			// New group
			if (firstWord[0] == 'g') {
				fgets(line, sizeof(line), f);

				std::vector<std::string> tokens = Utils::split(std::string(line), ' ');

				std::string back = tokens.back();

				// Don't care about default group
				if (back == "default") {
					continue;
				}

				// Name of object is last in the line
				groupName = back.substr(0, back.size()-1);

				std::cout << "Reading data for new group " << groupName;
				readData.start();
				 
				// Direct parent of object precedes name of object if line contains more words than "g <groupName>"
				int s = tokens.size();
				if (s > 2) {
					groupParent = tokens[s-2]; // tokens[-2]
					std::cout << "Parent! for " << groupName << " : " << groupParent << std::endl;
				} 

				continue;
				//goto nextline;
			}

			// Vertices
			if (strcmp(firstWord, "v") == 0) {
				++numOfPositions;

				fscanf(f, "%f %f %f", &a, &b, &c);

				positions.push_back(a);
				positions.push_back(b);
				positions.push_back(c);

				goto nextline;
			}

			// Texture Coords
			if (strcmp(firstWord, "vt") == 0) {
				++numOfTexCoords;

				fscanf(f, "%f %f", &a, &b);

				texCoords.push_back(a);
				texCoords.push_back(b);

				goto nextline;
			}

			// Vertex Normals 
			if (strcmp(firstWord, "vn") == 0) {
				++numOfNormals;

				fscanf(f, "%f %f %f", &a, &b, &c);

				normals.push_back(a);
				normals.push_back(b);
				normals.push_back(c);

				goto nextline;
			}

			// Faces
			if (firstWord[0] == 'f') {
				collectingFaces = true;

				fgets(line, sizeof(line), f);

				int i = 0;
				int k = 0;

				char c = line[i];
				char index[128];

				while (c != '\n' && c != '\0') {
					if (isdigit(c)){
						while (isdigit(c)){
							index[k++] = c;
							c = line[++i];
						}

						index[k] = '\0';
						k = 0;

						faceIndeces.push_back(std::stof(index) - 1);
					}

					c = line[++i];
				}
				
				/*
				// OLD WAY
				auto tokens = Utils::split("f" + std::string(line), ' ');

				for (int i = 1; i < 4; i++) {
					for (std::string f : Utils::split(tokens[i], '/')) {
						faceIndeces.push_back(std::stoi(f) - 1);
					}
				}
				*/

				continue;
			}

			std::cerr << "Unrecognized obj line: " << line << std::endl;

nextline:
			fgets(line, sizeof(line), f); // Eat rest of line
		}

		closeObject();

		fclose(f);
		
		for (auto p : meshes) {
			OBJMesh* m = p.second;
			std::cout << "Generating VBO for " << m->getMeshName() << std::endl;
			m->generateBuffers();
		}

		std::cout << "Finished loading file " << target << " (" << total.lap_s() << ")" << std::endl;

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
