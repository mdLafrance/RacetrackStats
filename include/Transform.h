#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

class Transform {
	glm::mat4x4 matrix;

	Transform* parent;

public:
	void translate(const glm::vec3& dp);
	void translate(const float& x, const float& y, const float& z);

	void rotate(const float& angle, const glm::vec3& dir);

	void scale(const glm::vec3& components);
	void scale(const float& x, const float& y, const float& z);

	void setParent(Transform* parent);

	glm::mat4x4 getViewProjectionMatrix();

	Transform();
	~Transform();
};
