// Maxime Lafrance 2020

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/transform.hpp>

class Transform {
	glm::mat4 matrix;
	bool updateMatrix; // Dirty flag for the matrix, matrix is cached on a getMatrix() call, and only recalculated if this is true

	glm::mat4 T;
	glm::mat4 R;
	glm::mat4 S;

	Transform* parent = nullptr;

public:
	void translate(const glm::vec3& dp);
	void setTranslation(const glm::vec3& dest);
	glm::mat4 Tmatrix() { return this->T; };

	// Rotation angle is in radians, for degrees multiply by Utils::DEG2RAD
	void rotate(const float& angle, const glm::vec3& dir);
	void setRotation(const float& angle, const glm::vec3& dir);
	glm::mat4 Rmatrix() { return this->R; };

	void scale(const glm::vec3& components);
	void setScale(const glm::vec3& components);
	glm::mat4 Smatrix() { return this->S; };

	void setParent(Transform* parent) { this->parent = parent; };
	Transform* getParent() { return this->parent; };

	void reset();

	glm::mat4x4 getMatrix(); // Transformation matrix including tranformations applied by any parents
	glm::mat4x4 getLocalMatrix(); // Transformation matrix of only local transforms. If no parent, these two will be the same

	// WCS vectors

	glm::vec3 position();

	glm::vec3 forward();
	glm::vec3 right();
	glm::vec3 up();

	Transform();
	~Transform();
};
