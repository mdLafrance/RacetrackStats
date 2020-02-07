#include <Light.h>

void Light::buildMatrix() {
	/*
		glm builds column-major matrices, so supply the appropriate columns outlined in resources/shaders/spec
	*/
	this->matrix = glm::mat3(
		this->color,
		this->x,
		{ this->type == LightType::DIRECTIONAL ? 0 : 1, 0, 0 } // glm::vec3
	);
}

glm::mat3 Light::getMatrix() {
	return this->matrix;
}

void Light::setColor(const glm::vec3& color) {
	this->color = color;
	this->buildMatrix();
}

void Light::setX(const glm::vec3& point_direction) {
	this->x = point_direction;
	this->buildMatrix();
}

Light::Light() {
	/*
		DEFAULT CONSTRUCTOR
	*/
	this->color = glm::vec3();
	this->x= glm::vec3();
	this->type = LightType::DIRECTIONAL;

	this->matrix = glm::mat4(1.0f);
}

Light::Light(const glm::vec3 color, const glm::vec3 point_direction, const LightType& type) {
	this->color = color;
	this->x = point_direction;
	this->type = type;

	this->buildMatrix();

	std::cout << "Created new " << (this->type == LightType::DIRECTIONAL ? "directional " : "point ") << "light: (" <<
		this->color[0] << ' ' <<
		this->color[1] << ' ' <<
		this->color[2] << ") (" <<

		this->x[0] << ' ' <<
		this->x[1] << ' ' <<
		this->x[2] << ')' << std::endl;
}
