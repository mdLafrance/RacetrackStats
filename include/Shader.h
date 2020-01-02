#pragma once

#include <string>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Shader {
	std::string target;

	int shaderProgram;

public:
	void bind();

	void setUniform4x4f(const std::string& name, const glm::mat4& target) const;

	Shader(const std::string& target);
	~Shader();
};
