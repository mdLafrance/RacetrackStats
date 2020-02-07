#pragma once

#include <iostream>

#include <glm/glm.hpp>

enum class LightType {DIRECTIONAL, POINT}; // spot?

class Light {
	/*
		Light class is a quick way of tracking simple lights in the scene.
		Lights with just color, position/direction, and two additional float parameters are sufficient for our purposes.
		Light data is assembled into a mat3 (as per resources/shaders/spec) and passed to diffuse shader.
	*/

	LightType type;

	glm::vec3 color;
	glm::vec3 x; // For directional light, this represents direction, for point, it is origin position

	glm::mat3 matrix;

	void buildMatrix();

public:
	glm::mat3 getMatrix();

	void setColor(const glm::vec3& color);
	void setX(const glm::vec3& point_direction);

	Light();
	Light(const glm::vec3 color, const glm::vec3 point_direction, const LightType& type);
	~Light() {};
};