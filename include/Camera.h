#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Transform.h>
#include <WorldState.h>

enum class CameraType {Perspective, Orthographic};

extern _WorldState WorldState;

class Camera {
	CameraType type;

	glm::mat4 proj;

	bool isMainCam;

public:
	Transform* transform;

	CameraType getType();

	glm::mat4 projectionViewMatrix();

	Camera(const float& fov, const float& aspect, const float& zMin, const float& zMax);
	Camera(const float& xMin, const float& xMax, const float& yMin, const float& yMax, const float& zMin, const float& zMax);
	~Camera();
};
