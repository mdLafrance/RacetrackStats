#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Transform.h>
#include <WorldState.h>

#define CAMERA_DEFAULT_FOV 44.5f
#define CAMERA_DEFAULT_FAR_CLIP_PLANE 1000.0f
#define CAMERA_DEFAULT_NEAR_CLIP_PLANE 0.12f

enum CameraType {Perspective, Orthographic};

extern _WorldState WorldState;

class Camera {
	CameraType type;

	glm::mat4 proj;

	float FOV;
	float farClipPlane;

public:
	void setFOV(const float& fov);
	void setFarClipPlane(const float& fcp);

	Transform* transform;

	CameraType getType();

	glm::mat4 projectionMatrix();
	glm::mat4 viewMatrix();
	inline glm::mat4 projectionViewMatrix() { return this->projectionMatrix() * this->viewMatrix(); };

	// Basic constructor, values are inferred from the global rendering settings
	Camera(const CameraType& type = CameraType::Perspective);

	// Perpective camera with arguments
	Camera(const float& fov, const float& zMin, const float& zMax);

	// Orthographic camera
	Camera(const float& xMin, const float& xMax, const float& yMin, const float& yMax, const float& zMin, const float& zMax);

	~Camera();
};
