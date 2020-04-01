#include "Transform.h"

glm::mat4x4 Transform::getMatrix() {
	if (this->updateMatrix){
		this->matrix = T * R * S;
		this->updateMatrix = false;
	}

	if (this->parent != nullptr) {
		return this->parent->getMatrix() * this->matrix;
	}
	else {
		return this->matrix;
	}
}

void Transform::setParent(Transform* parent){
	this->parent = parent;
}

void Transform::translate(const glm::vec3& dp){
	this->updateMatrix = true;
	this->T *= glm::translate(dp);
}

void Transform::setTranslation(const glm::vec3& dest){
	this->updateMatrix = true;
	this->T = glm::translate(dest);
}

void Transform::rotate(const float& angle, const glm::vec3& dir){
	this->updateMatrix = true;
	this->R = glm::rotate(angle, dir) * this->R;
}

void Transform::setRotation(const float& angle, const glm::vec3& dir){
	this->updateMatrix = true;
	this->R = glm::rotate(angle, dir);
}

void Transform::scale(const glm::vec3& components){
	this->updateMatrix = true;
	this->S *= glm::scale(components);
}

void Transform::setScale(const glm::vec3& components){
	this->updateMatrix = true;
	this->S = glm::scale(components);
}

glm::vec3 Transform::forward() {
	glm::vec4 k = this->getMatrix() * glm::vec4(0,0,-1,0);

	return glm::vec3(k);
}

glm::vec3 Transform::right() {
	glm::vec4 k = this->getMatrix() * glm::vec4(1, 0, 0, 0);

	return glm::vec3(k);
}

glm::vec3 Transform::up() {
	glm::vec4 k = this->getMatrix() * glm::vec4(0, 1, 0, 0);

	return glm::vec3(k);
}

glm::vec3 Transform::position(){
	return glm::vec3(glm::column(this->T, 3));
}

void Transform::reset() {
	this->matrix = glm::mat4(1.0f);

	this->T = glm::mat4(1.0f);
	this->R = glm::mat4(1.0f);
	this->S = glm::mat4(1.0f);

	this->updateMatrix = false;
}

Transform::Transform() {
	this->reset();

	this->parent = nullptr;
}

Transform::~Transform(){}
