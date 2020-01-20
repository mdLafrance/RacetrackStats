#include "Transform.h"

glm::mat4x4 Transform::getViewProjectionMatrix() {
	if (this->parent != nullptr) {
		return this->parent->getViewProjectionMatrix() * this->matrix;
	}
	else {
		return this->matrix;
	}
}

void Transform::setParent(Transform* parent){
	this->parent = parent;
}

Transform::Transform() {
	this->matrix = glm::mat4x4(1.0f);
	this->parent = nullptr;
}

Transform::~Transform(){}
