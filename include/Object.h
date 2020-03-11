#pragma once

#include <string>

#include <OBJ.h>
#include <Material.h>
#include <Transform.h>

// A struct to associate meshes with their textures and transforms

struct Object {
	std::string name;

	Transform* transform;
	OBJMesh* mesh;
	Material* material;

	Object(const std::string& name);
	~Object();
};