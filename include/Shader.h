#pragma once

#include <string>
#include <iostream>
#include <fstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <Utils.h>

class Shader {
	std::string target;

	unsigned int shaderProgram;

public:
	void bind();

	void setUniform4x4f(const std::string& name, const glm::mat4& target) const;

	unsigned int programID();

	Shader(const std::string& vertexTarget, const std::string& fragmentTarget);
	~Shader();
};
