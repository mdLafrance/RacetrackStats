#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Transform.h>
#include <WorldState.h>

#define CAMERA_DEFAULT_FOV 44.5f
#define CAMERA_DEFAULT_FAR_CLIP_PLANE 1000.0f
#define CAMERA_DEFAULT_NEAR_CLIP_PLANE 0.2f
#define CAMERA_DEFAULT_ORTHO_SCREEN_SCALE 0.015f

extern _WorldState WorldState;

enum CameraType { Perspective, Orthographic };

class Camera {
	CameraType type;

	float nearClipPlane, farClipPlane;

	float FOV = -1; // Perspective specific
	float size, depth; // Ortho specific 

public:
	CameraType getType() { return this->type; };

	void setFOV(const float& fov) { this->FOV = fov; };
	float getFOV() { return this->FOV; };

	void setSize(const float& s) { this->size = s; };
	void setDepth(const float& d) { this->depth = d; };

	float getSize() { return this->size; };
	float getDepth() { return this->depth; };

	void setNearClipPlane(const float& ncp) { this->nearClipPlane = ncp; };
	void setFarClipPlane(const float& fcp) { this->farClipPlane = fcp; };

	Transform* transform;

	glm::mat4 projectionMatrix();
	glm::mat4 viewMatrix();
	inline glm::mat4 projectionViewMatrix() { return this->projectionMatrix() * this->viewMatrix(); };

	// Simple constructor, infers values based on the renderers settings
	Camera(const CameraType& type = Perspective);

	// Perpective constructor
	Camera(const float& fov, const float& zMin, const float& zMax);

	// Ortho constructor
	Camera(const float& size, const float& depth);

	~Camera() { delete this->transform; };
};
