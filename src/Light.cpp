#include <Light.h>

void Light::buildMatrix() {
	/*
		glm builds column-major matrices, so supply the appropriate columns as defined in Light.h
	*/
	this->matrix = glm::mat3(
		this->color,
		this->x,
		{ this->t, this->i, this->k } // glm::vec3
	);
}

glm::mat3 Light::getMatrix() {
	// NOTE: Matrix instead gets rebuilt when values are changed, since this function is run very frequently
	return this->matrix;
}

void Light::setColor(const glm::vec3& color) {
	this->color = color;
	this->buildMatrix();
}

void Light::setPointDirection(const glm::vec3& point_direction) {
	this->x = point_direction;
	this->buildMatrix();
}

void Light::setIntensity(const float& i) {
	this->i = i;
	this->buildMatrix();
}

void Light::setK(const float& k) {
	this->k = k;
	this->buildMatrix();
}

Light::Light() {
	this->type = LightType::DIRECTIONAL;
	this->color = glm::vec3();
	this->x= glm::vec3();

	this->t = 0;
	this->i = DEFAULT_LIGHT_INTENSITY;
	this->k = DEFAULT_POINT_FALLOFF;

	this->matrix = glm::mat3(0.0f);
}

Light::Light(const LightType& type) {
	this->type = type;
	this->color = glm::vec3();
	this->x = glm::vec3();

	this->t = type == LightType::DIRECTIONAL ? 0 : 1;
	this->i = DEFAULT_LIGHT_INTENSITY;
	this->k = DEFAULT_POINT_FALLOFF;

	this->matrix = glm::mat3(0.0f);
}

Light::Light(const LightType& type, const glm::vec3 point_direction, const glm::vec3 color, const float& i){
	this->type = type;
	this->x = point_direction;
	this->color = color;

	this->t = type == LightType::DIRECTIONAL ? 0 : 1;
	this->i = i;
	this->k = DEFAULT_POINT_FALLOFF;

	this->buildMatrix();
}
