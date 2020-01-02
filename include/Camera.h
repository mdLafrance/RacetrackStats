#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
	glm::mat4 view;
	glm::mat4 proj;

	bool isMainCam;

public:
	void updateProjectionMatrix(const float& xMin, const float& xMax, const float& yMin, const float& yMax, const float& zMin, const float& zMax);

	void translate(const float& x, const float& y, const float& z);

	glm::mat4 projectionViewMatrix();

	Camera(const float& xMin, const float& xMax, const float& yMin, const float& yMax, const float& zMin, const float& zMax);
	~Camera();
};
