#include <Camera.h>

void Camera::updateProjectionMatrix(const float& xMin, const float& xMax, const float& yMin, const float& yMax, const float& zMin, const float& zMax) {
	this->proj = glm::ortho(xMin, xMax, yMin, yMax, zMin, zMax);
}

void Camera::translate(const float& x, const float& y, const float& z) {
	this->view = glm::translate(this->view, glm::vec3(-1 * x, -1 * y, -1 * z));
}

glm::mat4 Camera::projectionViewMatrix() {
	return this->proj * this->view;
}

Camera::Camera(const float& xMin, const float& xMax, const float& yMin, const float& yMax, const float& zMin, const float& zMax) {
	this->proj = glm::ortho(xMin, xMax, yMin, yMax, zMin, zMax);
	this->view = glm::mat4(1.0f);

	this->isMainCam = false;
}

Camera::~Camera() {
}
