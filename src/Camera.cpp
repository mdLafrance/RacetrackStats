#include <Camera.h>

glm::mat4 Camera::projectionMatrix() {
	if (this->type == Perspective) {
		return glm::perspective(this->FOV, (float)WorldState.rendererX / (float)WorldState.rendererY, this->z_min, this->z_max);
	}
	else if (this->type == Orthographic) {
		return glm::ortho(this->x_min, this->x_max, this->y_min, this->y_max, this->z_min, this->z_max);
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
	if (type == Perspective) {
		this->FOV = CAMERA_DEFAULT_FOV;
	}

	if (type == Orthographic) {
		float halfRendererX = WorldState.rendererX / 2;
		float halfRendererY = WorldState.rendererY / 2;

		this->x_min = -halfRendererX;
		this->x_max = halfRendererX;
		this->y_min = -halfRendererY;
		this->y_max = halfRendererY;
	}

	this->type = type;
	this->z_min = CAMERA_DEFAULT_NEAR_CLIP_PLANE;
	this->z_max = CAMERA_DEFAULT_FAR_CLIP_PLANE;

	this->transform = new Transform();
}

// Perpective constructor
Camera::Camera(const float& fov, const float& zMin, const float& zMax) {
	this->FOV = fov;
	this->z_min = zMin;
	this->z_max = zMax;

	this->transform = new Transform();
};

// Ortho constructor
Camera::Camera(const float& x_min, const float& x_max, const float& y_min, const float& y_max, const float& z_min, const float& z_max) {
	this->x_min = x_min;
	this->x_max = x_max;
	this->y_min = y_min;
	this->y_max = y_max;
	this->z_min = z_min;
	this->z_max = z_max;

	this->transform = new Transform();
}
