#include <iostream>
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stb/stb_image.h>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif // STBI_IMAGE_IMPLEMENTATION

class Skybox {
    unsigned int cubeMapID;

    unsigned int VAO, VBO;

public:
    void bind();
    void draw(const glm::vec3& cameraCenter = glm::vec3());

    unsigned int getId();

    // Create new cubemap using the (six) paths to images passed in in the vector to the constructor.
    // Faces should be passed in order as: +x, x, +y, y, +z, z
    Skybox(const std::vector<std::string>& faces);
    ~Skybox();
};

// Vertex positions for a basic cube
static const float cubeVertices[108] = {
	-100.0, -100.0, 100.0, 100.0, -100.0, 100.0, -100.0, 100.0, 100.0,
	- 100.0, 100.0, 100.0, 100.0, -100.0, 100.0, 100.0, 100.0, 100.0,
	- 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, -100.0, 100.0, -100.0,
	- 100.0, 100.0, -100.0, 100.0, 100.0, 100.0, 100.0, 100.0, -100.0,
	- 100.0, 100.0, -100.0, 100.0, 100.0, -100.0, -100.0, -100.0, -100.0,
	- 100.0, -100.0, -100.0, 100.0, 100.0, -100.0, 100.0, -100.0, -100.0,
	- 100.0, -100.0, -100.0, 100.0, -100.0, -100.0, -100.0, -100.0, 100.0,
	- 100.0, -100.0, 100.0, 100.0, -100.0, -100.0, 100.0, -100.0, 100.0,
	100.0, -100.0, 100.0, 100.0, -100.0, -100.0, 100.0, 100.0, 100.0,
	100.0, 100.0, 100.0, 100.0, -100.0, -100.0, 100.0, 100.0, -100.0,
	- 100.0, -100.0, -100.0, -100.0, -100.0, 100.0, -100.0, 100.0, -100.0,
	- 100.0, 100.0, -100.0, -100.0, -100.0, 100.0, -100.0, 100.0, 100.0,
};