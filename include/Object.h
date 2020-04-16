#pragma once

#include <string>

#include <OBJ.h>
#include <Material.h>
#include <Transform.h>

// A struct to associate meshes with their textures and transforms
// In the future, this would be where additional information like colliders or other game componenets could be grouped together

struct Object {
	std::string name;

	Transform* transform;
	OBJMesh* mesh;

	Object(const std::string& name);
	~Object();
};