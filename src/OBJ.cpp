#include <OBJ.h>

// inline void getFirstWord(const char* s, char* dest){
// 	int i = 0;

// 	while (s[i] != ' ' && s[i] != '\0'){
// 		*(dest + i) = s[i];
// 		i += 1;
// 	}

// 	*(dest+i) = '\0';
// }

namespace OBJ
{
	std::vector<OBJMesh*> load(const std::string& target) {
		FILE* f;

		f = fopen(target.c_str(), "r");

		if (!f) {
			std::cerr << "Couldn't Open File " << target << " For OBJ Loading" << std::endl;
			return std::vector<OBJMesh*>();
		}

		Utils::StopWatch writeVertex, readData, total;

		total.start();

		Utils::FileInfo fi = Utils::getFileInfo(target);

		const char* line_cstr;
		char firstWord[128];
		char line[1024];

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

		std::vector<OBJ::FaceMaterials> faceMaterials;

		std::string groupName;
		std::string groupParent;

		OBJMesh* currentObject;

		int currentPositionIndex;
		int currentNormalIndex;
		int currentTexCoordIndex;

		std::vector<OBJMesh*> meshes;

		// Dummy values
		int dataSize = 1;
		float* data = new float[1];

		auto closeObject = [&]() {
			std::cout << " (" << readData.lap_s() << ")" << std::endl;

			writeVertex.start();

			collectingFaces = false;

			int numOfFaces = faceIndeces.size() / 9; // 3 components per 3 verteces per face

			if (!faceMaterials.empty()) {
				faceMaterials.back().range[1] = numOfFaces;
			}

			std::string fullName = fi.file + '.' + groupName;
			std::string fullParentName = groupParent.size() == 0 ? "" : fi.file + '.' + groupParent;

			std::cout << "Writing vertex data for " << groupName << " (" << numOfFaces << " tris, ";

			float* data = new float[3 * 8 * numOfFaces];

			for (int i = 0; i < 3 * numOfFaces; i++) {
				// add position triplet for each face
				currentPositionIndex = faceIndeces[(3 * i) + 0];
				*(data + (8 * i) + 0) = positions[(3 * currentPositionIndex) + 0];
				*(data + (8 * i) + 1) = positions[(3 * currentPositionIndex) + 1];
				*(data + (8 * i) + 2) = positions[(3 * currentPositionIndex) + 2];

				// add normals triplet for each face
				currentNormalIndex = faceIndeces[(3 * i) + 2];
				*(data + (8 * i) + 3) = normals[(3 * currentNormalIndex) + 0];
				*(data + (8 * i) + 4) = normals[(3 * currentNormalIndex) + 1];
				*(data + (8 * i) + 5) = normals[(3 * currentNormalIndex) + 2];

				// add tex coordinates pair for each face
				currentTexCoordIndex = faceIndeces[(3 * i) + 1];
				*(data + (8 * i) + 6) = texCoords[(2 * currentTexCoordIndex) + 0];
				*(data + (8 * i) + 7) = texCoords[(2 * currentTexCoordIndex) + 1];
			}

			currentObject = new OBJMesh(
				fullName, 
				fullParentName, 
				target,
				numOfFaces,
				data,
				faceMaterials
			);

			// Add generated object to map
			meshes.push_back(currentObject);

			// Cleanup for next mesh

			numOfPositions = 0;
			numOfTexCoords = 0;
			numOfNormals = 0;

			faceIndeces.clear();

			faceMaterials.clear();

			std::cout << writeVertex.lap_s() << ")" << std::endl;
		};

		long lines = 0;

		while (fscanf(f, "%s", firstWord) != EOF) {
			++lines;

			if (firstWord[0] == '\n' || firstWord[0] == '#' || firstWord[0] == 's') {
				goto nextline;
			}

			if (strcmp(firstWord, "mtllib") == 0) {
				fscanf(f, "%s", line); 
				materialLibrary = std::string(line);
				std::cout << "File using material library " << materialLibrary << std::endl;
				goto nextline;
			}

			if (strcmp(firstWord, "usemtl") == 0) {
				fscanf(f, "%s", line); 

				int currentNumOfFaces = faceIndeces.size() / 9; // 3 components per 3 vertices per face

				// If this isn't the last material defined on these faces, save the end to the range of the last material
				if (!faceMaterials.empty()) {
					faceMaterials.back().range[1] = currentNumOfFaces;
				}

				faceMaterials.push_back({ Utils::getFileInfo(materialLibrary).file + '.' + std::string(line), currentNumOfFaces, -1 });

				goto nextline;
			}

			// New group
			if (firstWord[0] == 'g') {
				// If we were previously writing to an object, save the vertices and data for that object.
				if (faceIndeces.size() != 0) {
					closeObject();
				}

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
					std::cout << "Parent for " << groupName << " : " << groupParent << std::endl;
				} 

				continue;
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

				// Find all face indeces separated by non numeric characters
				while (c != '\n' && c != '\0') {
					if (isdigit(c)){
						while (isdigit(c)){
							index[k++] = c;
							c = line[++i];
						}

						index[k] = '\0';
						k = 0;

						faceIndeces.push_back(std::stof(index) - 1); // Indeces in .obj are 1-indexed, convert to 0-index
					}

					c = line[++i];
				}

				continue;
			}

			std::cerr << "Unrecognized obj line: " << line << std::endl;

nextline:
			fgets(line, sizeof(line), f); // Eat rest of line
		}

		closeObject();

		fclose(f);

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
	// TODO: use EBO instead?

	// Generate and bind VAO for this mesh
	glGenVertexArrays(1, &this->VAO);
	glBindVertexArray(this->VAO);

	// Generate vertex attribute buffer
	int lenOfVBO = sizeof(float) * 3 * 8 * this->numberOfFaces;
	glGenBuffers(1, &this->VBO);
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	glBufferData(GL_ARRAY_BUFFER, lenOfVBO, this->vertexData, GL_STATIC_DRAW);

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

	// At this point, no need for vertex data any longer since it is already saved in the VAO
	delete[] this->vertexData;
	this->vertexData = nullptr;

	this->loaded = true;
}

void OBJMesh::draw() {
	glDrawArrays(GL_TRIANGLES, 0, 3 * this->numberOfFaces);
}

void OBJMesh::drawRange(int start, int count) {
	// NOTE: Using assert's here to squeeze out a bit more performance, they won't get compiled in release (see top of main.cpp)
	assert(start >= 0 && "Negative start value given");
	assert(count <= this->numberOfFaces && "Too many faces given");
	assert(this->loaded && "Mesh is not loaded (call generateBuffers)");
	assert((count <= this->numberOfFaces - start) && ("Attempting to draw too many faces"));
	
	glDrawArrays(GL_TRIANGLES, 3 * start, 3 * count); // each face is defined by three elements (face 0 = [0,2], face 1 = [3,6] etc.)
}

OBJMesh::OBJMesh(const std::string& meshName, const std::string& parent, const std::string& origin, const int& numberOfTriangles, float* data, const std::vector<OBJ::FaceMaterials>& faceMaterials) {
	this->meshName = meshName;
	this->defaultMaterialName = "FOO";
	this->defaultParent = parent;
	this->origin = origin;

	this->numberOfFaces = numberOfTriangles;
	this->vertexData = data;

	this->faceMaterials = faceMaterials;
}

OBJMesh::~OBJMesh() {
	glDeleteBuffers(1, &this->VBO);
	glDeleteVertexArrays(1, &this->VAO);

	// If for some reason vertex data was not 'transferred' into VAO, delete it now
	if (!(this->vertexData == nullptr)){
		delete[] this->vertexData;
	}
}
