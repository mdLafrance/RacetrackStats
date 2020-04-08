#pragma once

#include <iostream>

#include <glm/glm.hpp>

#define DEFAULT_LIGHT_INTENSITY 1
#define DEFAULT_POINT_FALLOFF 1

enum class LightType {DIRECTIONAL, POINT}; // spot?

// Currently only directional is implemented

// Lights work by passing a 3x3 matrix to glsl as a uniform to keep the data compact
// First column of the matrix is the colour (rgb)
// Second column of the matrix is either the direction of the light (directional) or the origin of the light (point)
// Third column holds three values; t, i, k
// t is the type of light (0=directional)
// i is the intensity of the light
// k is an extra paramater (falloff for point light)

class Light {
	LightType type;

	glm::vec3 color;
	glm::vec3 x; // For directional light, this represents direction, for point, it is origin position

	// Three components available for parameters.
	// t -> type of light (0=directional, !0=point)
	// i -> light intensity
	// k -> for point: falloff speed of the function: 1-(x/(x-k))
	float t;
	float i;
	float k;

	bool matrixDirty = false;

	glm::mat3 matrix;

	void buildMatrix();

public:
	glm::mat3 getMatrix();

	void setColor(const glm::vec3& color);
	void setPointDirection(const glm::vec3& point_direction);
	void setIntensity(const float& i);
	void setK(const float& k);

	Light();
	Light(const LightType& type);
	Light(const LightType& type, const glm::vec3 point_direction, const glm::vec3 color, const float& i);
	~Light() {};
};