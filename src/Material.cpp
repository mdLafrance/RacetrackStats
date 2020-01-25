#include <Material.h>

std::map<std::string, Material*> Material::load(const std::string& target)
{
	std::map<std::string, Material*> materials;

	std::string line;
	std::vector<std::string> tokens;
	Material* current = nullptr;

	std::ifstream f(target);
	if (f.is_open()) {
		while (getline(f, line)) {
			// # indicates comment
			if (line[0] == '#') continue;

			tokens = Utils::split(line, ' ');

			// Empty line
			if (tokens.size() == 0) {
				continue;
			}

			std::string lineType = tokens[0];

			// Start recording information for given material
			if (lineType == "newmtl") {
				// If a new material is encountered while one is being recorded, finish the current one.
				if (current) {
					materials[current->name] = current;
				}
				std::cout << "Generating Material " << tokens[1] << std::endl;
				current = new Material(tokens[1], "default", "default");
			} else if (lineType == "map_Kd") { // DIFFUSE MAP
			} else if (lineType == "map_Ks"){ // SPEC MAP
			} else if (lineType == "norm") { // NORMAL MAP
			} else if (lineType == "illum") { // Illumination
				current->illum = std::atof(tokens[1].c_str());
			} else if (lineType == "Ka") { // Ambient 
				current->Ka = glm::vec3();
			} else if (lineType == "Kd"){ // Diffuse
				current->Kd = glm::vec3();
			} else if (lineType == "Ks"){ // Specular color
				current->Ks = glm::vec3();
			} else if (lineType == "Ns"){ // Specular exp
				current->Ns = std::atof(tokens[1].c_str());
			} else if (lineType == "Tr"){ // Transparency value, 1 is transparent
				current->Tr = std::atof(tokens[1].c_str());
			}
		}
	} else {
		std::cerr << "Couldn't open .mtl file: " << target << std::endl;
		return std::map<std::string, Material*>();
	}

	f.close();

	// Save last material
	if (current != nullptr) {
		materials[current->name] = current;
	}

	return materials;
}


Material::Material(const std::string& name, const std::string& texture, const std::string& shader) {
	this->name = name;
}

Material::~Material() {
}


