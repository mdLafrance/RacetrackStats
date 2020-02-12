#pragma once

#include <string>
#include <iostream>
#include <fstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <Light.h>
#include <Utils.h>

class Shader {
	unsigned int shaderProgram;

	std::string target;

public:
	void bind();

	void setUniformMatrix4fv(const std::string& name, const glm::mat4& target) const;
	void setUniform3fv(const std::string& name, const glm::vec3& v) const;
	void setUniformf(const std::string& name, const float& f) const;

	void setLights(const int& count, const glm::mat3* m0);

	unsigned int programID();

	Shader(const std::string& vertexTarget, const std::string& fragmentTarget);
	~Shader();
};
