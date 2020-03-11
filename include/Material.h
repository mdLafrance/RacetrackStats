#pragma once

#include <map>
#include <fstream>
#include <cstdint>

#include <glm/glm.hpp>

#include <Texture.h>
#include <Shader.h>
#include <Utils.h>

// Flag bits, matched in shaders where applicable

#define TEXTURE_LOCATION_map_Kd 0
#define TEXTURE_LOCATION_map_Ka 1
#define TEXTURE_LOCATION_map_Ks 2
#define TEXTURE_LOCATION_norm   3

#define MATERIAL_USE_map_Kd   1
#define MATERIAL_USE_map_Ks   1 << 1
#define MATERIAL_USE_map_norm 1 << 2
#define MATERIAL_USE_Ka       1 << 3
#define MATERIAL_USE_Kd       1 << 4
#define MATERIAL_USE_Ks       1 << 5
#define MATERIAL_USE_Ns       1 << 6
#define MATERIAL_USE_Tr       1 << 7

struct Material {

	std::string name; // Name of material
	std::string origin; // Path to material library this material originated from

	uint32_t flags;

	int illum;    // Illumination mode
	glm::vec3 Ka; // ambient
	glm::vec3 Kd; // diffuse
	glm::vec3 Ks; // spec
	float Ns;     // spec exponent
	float Tr;     // transparency, 1 = fully transparent

	Texture* map_Kd; // Diffuse map
	Texture* map_Ks; // Spec map
	Texture* norm; // Normal map 

	Shader* shader;

	inline void addFlag(const uint32_t& flag);
	inline void removeFlag(const uint32_t& flag);
	inline bool checkFlag(const uint32_t& flag);

	void bind();

	void setMVP(const glm::mat4& mvp);

	Material(const std::string& name);
	~Material();
}; 

namespace MTL
{
	std::map<std::string, Material*> load(const std::string& target);
}