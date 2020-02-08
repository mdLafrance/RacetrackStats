#include <Camera.h>

void Camera::updateProjectionMatrix(const float& xMin, const float& xMax, const float& yMin, const float& yMax, const float& zMin, const float& zMax) {
	this->proj = glm::perspective(70.0f, xMax/yMax, zMin, zMax);
}

glm::mat4 Camera::projectionViewMatrix() {
	return this->proj * this->transform->getMatrix();
}

Camera::Camera(const float& xMin, const float& xMax, const float& yMin, const float& yMax, const float& zMin, const float& zMax) {
	//this->proj = glm::ortho(xMin, xMax, yMin, yMax, zMin, zMax);
	this->proj = glm::perspective(45.0f, xMax/yMax, 1.0f, zMax);
	this->view = glm::mat4(1.0f);

	this->transform = new Transform();
}

Camera::~Camera() {
	delete this->transform;
}
