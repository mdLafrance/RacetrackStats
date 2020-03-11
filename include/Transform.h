#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/transform.hpp>

// TODO: internals of this need some work

class Transform {
	glm::mat4 matrix;

	glm::mat4 T;
	glm::mat4 R;
	glm::mat4 S;

	bool updateMatrix;

	Transform* parent;

public:
	void translate(const glm::vec3& dp);
	void setTranslation(const glm::vec3& dest);

	void rotate(const float& angle, const glm::vec3& dir);
	void setRotation(const float& angle, const glm::vec3& dir);

	void scale(const glm::vec3& components);
	void setScale(const glm::vec3& components);

	void setParent(Transform* parent);

	glm::mat4x4 getMatrix();

	glm::vec3 position();

	glm::vec3 forward();
	glm::vec3 right();
	glm::vec3 up();

	Transform();
	~Transform();
};
