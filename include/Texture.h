#pragma once
#include <iostream>
#include <string.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

#include <Utils.h>

class Texture {
	std::string name;

	int width;
	int height;
	int nrChannels;

	unsigned int ID;

	unsigned char* data;

public:
	void bind();

	std::string getName();
	unsigned int getID();
	void getWidthHeight(float& w, float& h);

	Texture(const std::string& target);
	~Texture();
};
