#pragma once

#include <iostream>
#include <fstream>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <Light.h>
#include <Utils.h>

class Shader {
	unsigned int shaderProgram;

public:
	void bind();

	void setUniform(const char* name, const float& val);
	void setUniform(const char* name, const unsigned int& val);
	void setUniform(const char* name, const glm::vec3& val);
	void setUniform(const char* name, const glm::mat4& val);

	void setLights(const int& count, const glm::mat3* m0);

	unsigned int programID() { return this->shaderProgram; };

	Shader(const std::string& vertexTarget, const std::string& fragmentTarget);
	~Shader();

	// GLSL doesn't natively support #include directive, so manually implement this if
	static std::string loadShaderSource(const std::string& target, const bool& followIncludes = true);
};
