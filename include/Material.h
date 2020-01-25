#pragma once

#include <map>
#include <fstream>

#include <glm/glm.hpp>
#include <Texture.h>
#include <Shader.h>

struct Material {
	std::string name;
	std::string origin;

	int illum;    // Illumination mode
	glm::vec3 Ka; // ambient
	glm::vec3 Kd; // diffuse
	glm::vec3 Ks; // spec
	float Ns;     // spec exponent
	float Tr;     // transparency, 1 = fully transparent

	Texture* map_Kd; // Diffuse map
	Texture* map_Ks; // Spec map
	Texture* norm; // Probably not gonna have time for this

	Shader* shader;

	void bind();

	static std::map<std::string, Material*> load(const std::string& target);

	void setMVP(const glm::mat4x4& MVP);

	Material(const std::string& name, const std::string& texture, const std::string& shader);
	~Material();
}; 
