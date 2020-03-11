#include <Camera.h>

CameraType Camera::getType() {
	return this->type;
}

glm::mat4 Camera::projectionViewMatrix() {
	glm::mat4 view;

	glm::vec3 x, y, z, e;
	
	e = this->transform->position();
	x = -this->transform->right();
	y = this->transform->up();
	z = this->transform->forward();

	view = {
		glm::vec4(x[0], y[0], z[0], 0),
		glm::vec4(x[1], y[1], z[1], 0),
		glm::vec4(x[2], y[2], z[2], 0),
		glm::vec4(-glm::dot(x, e), -glm::dot(y, e), -glm::dot(z, e), 1)

	};

	return this->proj * view;
}
void Camera::setPerspectiveProjMatrix(const float& fov, const float& aspect, const float& zMin, const float& zMax) {
	this->proj = glm::perspective(fov, aspect, zMin, zMax);
}

Camera::Camera(const float& fov, const float& aspect, const float& zMin, const float& zMax) {
	// Constructor for perspective camera

	this->type = CameraType::Perspective;

	this->proj = glm::perspective(45.0f, aspect, 1.0f, zMax);

	this->transform = new Transform();
}

Camera::Camera(const float& xMin, const float& xMax, const float& yMin, const float& yMax, const float& zMin, const float& zMax) {
	// Perspective for orthographic camera 

	this->type = CameraType::Orthographic;

	this->proj = glm::ortho(xMin, xMax, yMin, yMax, zMin, zMax);

	this->transform = new Transform();
}

Camera::~Camera() {
	delete this->transform;
}
