#include <CubeMap.h>

// UNDER CONSTRUCTION

void CubeMap::bind(){
    glBindTexture(GL_TEXTURE_CUBE_MAP, this->cubeMapID);
}

unsigned int CubeMap::getId(){
    return this->cubeMapID;
}

CubeMap::CubeMap(const std::vector<std::string>& faces) {
    std::cout << "Loading new CubeMap" << std::endl;

    int numOfFaces = faces.size();

    // just in case, clamp to 6 faces
    if (numOfFaces > 6){
        std::cerr << "Warning: Attempting to load too many faces into new CubeMap (" << numOfFaces << "). These faces will be discarded." << std::endl;
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
            std::cerr << "ERROR: Failed to load image [" << faces[i] << "] for new CubeMap." << std::endl;
        }

        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
