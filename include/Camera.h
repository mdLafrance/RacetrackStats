#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Transform.h>
#include <WorldState.h>

#define CAMERA_DEFAULT_FOV 45.0f
#define CAMERA_DEFAULT_FAR_CLIP_PLANE 1000.0f
#define CAMERA_DEFAULT_NEAR_CLIP_PLANE 0.1f

enum CameraType {Perspective, Orthographic};

extern _WorldState WorldState;

class Camera {
	CameraType type;

	glm::mat4 proj;

	bool isMainCam = false;

	float FOV;
	float farClipPlane;

public:
	void setFOV(const float& fov);
	void setFarClipPlane(const float& fcp);

	Transform* transform;

	CameraType getType();

	glm::mat4 projectionViewMatrix();

	// Basic constructor, values are inferred from the global rendering settings
	Camera(const CameraType& type = CameraType::Perspective);

	// Perpective camera with arguments
	Camera(const float& fov, const float& zMin, const float& zMax);

	// Orthographic camera
	Camera(const float& xMin, const float& xMax, const float& yMin, const float& yMax, const float& zMin, const float& zMax);

	~Camera();
};
