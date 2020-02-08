#include <Camera.h>

CameraType Camera::getType()
{
	return this->type;
}

glm::mat4 Camera::projectionViewMatrix() {
	return this->proj * this->transform->getMatrix();
}

Camera::Camera(const float& fov, const float& aspect, const float& zMin, const float& zMax) {
	this->type = CameraType::Perspective;

	this->proj = glm::perspective(45.0f, aspect, 1.0f, zMax);
	this->view = glm::mat4(1.0f);

	this->transform = new Transform();
}

Camera::Camera(const float& xMin, const float& xMax, const float& yMin, const float& yMax, const float& zMin, const float& zMax) {
	this->type = CameraType::Orthographic;

	this->proj = glm::ortho(xMin, xMax, yMin, yMax, zMin, zMax);
	this->view = glm::mat4(1.0f);

	this->transform = new Transform();
}

Camera::~Camera() {
	delete this->transform;
}
