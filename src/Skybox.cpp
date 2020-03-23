#include <Skybox.h>

void Skybox::bind(){
    glBindTexture(GL_TEXTURE_CUBE_MAP, this->cubeMapID);
    glBindVertexArray(this->VAO);
}

void Skybox::draw(){
    glBindTexture(GL_TEXTURE_CUBE_MAP, this->cubeMapID);
    glBindVertexArray(this->VAO);

    glDrawArrays(GL_TRIANGLES, 0, 36); // Number of tris for a minimal cube is 12, with 3 vertices each

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

unsigned int Skybox::getId(){
    return this->cubeMapID;
}

Skybox::Skybox(const std::vector<std::string>& faces) {
    std::cout << "Loading new Skybox" << std::endl;

    int numOfFaces = faces.size();

    // just in case, clamp to 6 faces
    if (numOfFaces > 6){
        std::cerr << "Warning: Attempting to load too many faces (" << numOfFaces << ") into new Skybox. These faces will be discarded." << std::endl;
        numOfFaces = 6;
    }

    glGenTextures(1, &this->cubeMapID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, this->cubeMapID);

    unsigned char *data;
    int width, height, nrChannels;

    // Generate each passed texture
    for (int i = 0; i < numOfFaces; i++){
        data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);

        if (data){
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            std::cout << "Loaded image " << faces[i] << std::endl;
        } else {
            std::cerr << "ERROR: Failed to load image [" << faces[i] << "] for new Skybox." << std::endl;
        }

        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    // Create basic cube mesh

    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);

    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 108, cubeVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3, 0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Skybox::~Skybox(){
	glDeleteBuffers(1, &this->VBO);
	glDeleteVertexArrays(1, &this->VAO);

	glDeleteTextures(1, &this->cubeMapID);
}