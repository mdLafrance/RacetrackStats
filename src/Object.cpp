#include "Object.h"

Object::Object(const std::string& name) {
	this->name = name;
	this->transform = new Transform();
}

Object::~Object() {
	delete this->transform;
}
