#include <Camera.h>

glm::mat4 Camera::projectionMatrix() {
	if (this->type == Perspective) {
		return glm::perspective(this->FOV, (float)WorldState.rendererX / (float)WorldState.rendererY, this->nearClipPlane, this->farClipPlane);
	}
	else if (this->type == Orthographic) {
		float w = this->size * WorldState.rendererX;
		float h = this->size * WorldState.rendererY;

		return glm::ortho(-w / 2, w / 2, -h / 2, h / 2, -this->depth/2, this->depth/2);
	}
}

glm::mat4 Camera::viewMatrix() {
	glm::mat4 camTransform = this->transform->getMatrix();

	glm::vec3 e = glm::vec3(glm::column(camTransform, 3));
	glm::vec3 x = camTransform * glm::vec4(1, 0, 0, 0);
	glm::vec3 y = camTransform * glm::vec4(0, 1, 0, 0);
	glm::vec3 z = camTransform * glm::vec4(0, 0, 1, 0);

	return glm::mat4(
		glm::vec4(x[0], y[0], z[0], 0),
		glm::vec4(x[1], y[1], z[1], 0),
		glm::vec4(x[2], y[2], z[2], 0),
		glm::vec4(-glm::dot(x, e), -glm::dot(y, e), -glm::dot(z, e), 1)

	);
}

// Simple constructor, infers values based on the renderers settings
Camera::Camera(const CameraType& type) {
	this->type = type;
	this->transform = new Transform();

	if (type == Perspective) {
		this->FOV = CAMERA_DEFAULT_FOV;
		this->nearClipPlane = CAMERA_DEFAULT_NEAR_CLIP_PLANE;
		this->farClipPlane = CAMERA_DEFAULT_FAR_CLIP_PLANE;
	}

	if (type == Orthographic) {
		this->FOV = CAMERA_DEFAULT_FOV;
		this->size = CAMERA_DEFAULT_ORTHO_SCREEN_SCALE;
		this->depth = CAMERA_DEFAULT_FAR_CLIP_PLANE - CAMERA_DEFAULT_NEAR_CLIP_PLANE;
	}
}

// Perpective constructor
Camera::Camera(const float& fov, const float& zMin, const float& zMax) {
	this->type = Perspective;
	this->transform = new Transform();

	this->FOV = fov;
	this->nearClipPlane = zMin;
	this->farClipPlane = zMax;
};

// Ortho constructor
Camera::Camera(const float& size, const float& depth) {
	this->type = Orthographic;
	this->transform = new Transform();

	this->size = size;
	this->depth = depth;

	this->FOV = CAMERA_DEFAULT_FOV;
}
