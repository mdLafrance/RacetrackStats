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

void Texture::getWidthHeight(float& w, float& h) {
	w = this->width;
	h = this->height;
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

	std::string fname;

	if (Utils::hasEnding(target, "dds")){
		fname = (target.substr(0, target.size()-3) + "png");
		std::cerr << "Attempting to load unsupported dds file, will now try to load png mirror: " << fname << std::endl;
	} else {
		fname = target;
	}

	stbi_set_flip_vertically_on_load(true);

	this->data = stbi_load(fname.c_str(), &this->width, &this->height, &this->nrChannels, STBI_rgb_alpha);

	this->name = Utils::getFileNameNoExtension(fname);

	glGenTextures(1, &this->ID);
	glBindTexture(GL_TEXTURE_2D, this->ID); 

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, this->data);
		glGenerateMipmap(GL_TEXTURE_2D);
		std::cout << "Loaded Texture: " << fname << std::endl;
	}
	else {
		std::cerr << "ERROR: Couldn't load texture " << fname << std::endl;
	}

	stbi_image_free(this->data);
	glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture() {
	glDeleteTextures(1, &this->ID);
}
