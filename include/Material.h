#pragma once

#include <map>
#include <fstream>

#include <Texture.h>
#include <Shader.h>

struct Material {
	std::string name;
	std::string texture;
	std::string shader;

	static std::map<std::string, Material*> load(const std::string& target);

	Material(const std::string& name, const std::string& texture, const std::string& shader);
	~Material();
}; 
