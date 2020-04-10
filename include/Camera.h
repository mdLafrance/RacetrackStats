#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Transform.h>
#include <WorldState.h>

#define CAMERA_DEFAULT_FOV 44.5f
#define CAMERA_DEFAULT_FAR_CLIP_PLANE 1000.0f
#define CAMERA_DEFAULT_NEAR_CLIP_PLANE 0.12f

extern _WorldState WorldState;

enum CameraType { Perspective, Orthographic };

class Camera {
	float FOV = -1;

	float x_min, x_max, y_min, y_max, z_min, z_max;

	CameraType type;

public:
	CameraType getType() { return this->type; };

	void setFOV(const float& fov) { this->FOV = fov; };
	void setNearClipPlane(const float& ncp) { this->z_min = ncp; };
	void setFarClipPlane(const float& fcp) { this->z_max = fcp; };

	Transform* transform;

	glm::mat4 projectionMatrix();
	glm::mat4 viewMatrix();
	inline glm::mat4 projectionViewMatrix() { return this->projectionMatrix() * this->viewMatrix(); };

	// Simple constructor, infers values based on the renderers settings
	Camera(const CameraType& type = Perspective);

	// Perpective constructor
	Camera(const float& fov, const float& zMin, const float& zMax);

	// Ortho constructor
	Camera(const float& x_min, const float& x_max, const float& y_min, const float& y_max, const float& z_min, const float& z_max);

	~Camera() { delete this->transform; };
};
