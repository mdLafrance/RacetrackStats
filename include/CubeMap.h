#include <stb/stb_image.h>

#include <Texture.h>

class CubeMap {
    unsigned int cubeMapID;

    // Draw a quad over the scene 
    unsigned int VAO, VBO;

public:
    void bind();
    void draw();

    unsigned int getId();

    // Create new cubemap using the (six) paths to images passed in in the vector to the constructor.
    // Faces should be passed in order as: +x, -x, +y, -y, +z, -z
    CubeMap(const std::vector<std::string>& faces);
    ~CubeMap();
};