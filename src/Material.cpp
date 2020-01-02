#include <Material.h>

std::map<std::string, Material*> Material::load(const std::string& target)
{
	std::map<std::string, Material*> materials;

	std::string line;
	std::vector<std::string> tokens;
	Material* current = nullptr;

	std::ifstream f(target);
	if (f.is_open()) {
		std::cout << "Loading materials from " << target << std::endl;

		while (getline(f, line)) {
			tokens = Utils::split(line, ' ');

			if (tokens.size() == 0) {
				continue;
			}

			if (tokens[0] == "newmtl") {
				if (current) {
					materials[current->name] = current;
				}
				std::cout << "GENERATING MATERIAL " << tokens[1] << std::endl;
				current = new Material(tokens[1], "default", "default");
			}

			if (tokens[0] == "map_Kd") {
				if (current) {
					current->texture = Utils::getFileNameNoExtension(tokens[1]);
				}
			}
		}

	}
	else {
		std::cerr << "Couldn't open .mtl file: " << target << std::endl;
	}

	if (current != nullptr) {
		materials[current->name] = current;
	}

	for (auto m : materials) {
		std::cout << "Generated " << m.first << " with texture: " << m.second->texture << std::endl;
	}

	f.close();

	return materials;
}


Material::Material(const std::string& name, const std::string& texture, const std::string& shader) {
	this->name = name;
	this->texture = texture;
	this->shader = shader;
}

Material::~Material() {
}


