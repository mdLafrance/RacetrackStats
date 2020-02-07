#include <Texture.h>

void Texture::bind() {
	glBindTexture(GL_TEXTURE_2D, this->ID);
}

std::string Texture::getName()
{
	return this->name;
}

unsigned int Texture::getID() {
	return this->ID;
}

Texture::Texture(const std::string& target) {
	if (target == "default") {
		this->width = 0;
		this->height = 0;
		this->nrChannels = 0;
		this->data = nullptr;
		this->name = "default";
		this->ID = 0;
		return;
	}

	stbi_set_flip_vertically_on_load(true);

	this->data = stbi_load(target.c_str(), &this->width, &this->height, &this->nrChannels, 0);

	this->name = Utils::getFileNameNoExtension(target);

	glGenTextures(1, &this->ID);
	glBindTexture(GL_TEXTURE_2D, this->ID); 

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->width, this->height, 0, GL_RGB, GL_UNSIGNED_BYTE, this->data);
		glGenerateMipmap(GL_TEXTURE_2D);
		std::cout << "Loaded Texture: " << target << std::endl;
	}
	else {
		std::cout << "ERROR: Couldn't load texture " << target << std::endl;
	}

	stbi_image_free(this->data);
	glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture() {
	glDeleteTextures(1, &this->ID);
}
