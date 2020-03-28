#pragma once
#include <iostream>
#include <string.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stb/stb_image.h>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif // STBI_IMAGE_IMPLEMENTATION

#include <Utils.h>

class Texture {
	std::string name;

	int nrChannels;

	int width;
	int height;

	unsigned int ID;

	unsigned char* data;

public:
	void bind();

	std::string getName();
	unsigned int getID();
	void getWidthHeight(int* const w, int* const h);

	Texture(const std::string& target, const bool& alphaChannel = true, const bool& flipVertical = true, int* const w = nullptr, int* const height = nullptr);
	~Texture();
};
