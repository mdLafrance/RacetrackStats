#pragma once

#include <map>
#include <fstream>

#include <glm/glm.hpp>

#include <Texture.h>
#include <Shader.h>
#include <Utils.h>

struct Material {
	std::string name; // Name of material
	std::string origin; // Path to material library this material originated from

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

	void bind();

	static std::map<std::string, Material*> load(const std::string& target);

	Material(const std::string& name, const std::string& texture, const std::string& shader);
	~Material();
}; 
