#include "Transform.h"

glm::mat4x4 Transform::getMatrix() {
	if (this->parent != nullptr) {
		return this->parent->getMatrix() * this->matrix;
	}
	else {
		return this->matrix;
	}
}

Transform::Transform() {
	this->matrix = glm::mat4x4(1.0f);
}

Transform::~Transform(){}
