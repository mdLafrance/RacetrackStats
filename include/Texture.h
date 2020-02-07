#pragma once
#include <iostream>
#include <string.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

#include <Utils.h>

class Texture {
	int width;
	int height;
	int nrChannels;

	std::string name;

	unsigned int ID;

	unsigned char* data;

public:
	void bind();

	std::string getName();

	unsigned int getID();

	Texture(const std::string& target);
	~Texture();
};
