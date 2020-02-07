#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Transform.h>
#include <WorldState.h>

extern _WorldState WorldState;

class Camera {
	glm::mat4 view;
	glm::mat4 proj;

	bool isMainCam;

public:
	Transform* transform;

	void updateProjectionMatrix(const float& xMin, const float& xMax, const float& yMin, const float& yMax, const float& zMin, const float& zMax);

	glm::mat4 projectionViewMatrix();

	glm::vec3 forward();

	Camera(const float& xMin, const float& xMax, const float& yMin, const float& yMax, const float& zMin, const float& zMax);
	~Camera();
};
