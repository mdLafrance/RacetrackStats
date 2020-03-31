#include <Material.h>

namespace MTL 
{
	std::vector<Material*> load(const std::string& target) {
		// TODO: might switch this to faster c-style file reading, but mtl files are usually small so this is on the backburner

		std::vector<Material*> materials;

		std::string line;
		std::vector<std::string> tokens;
		Material* current = nullptr;

		Utils::FileInfo fi = Utils::getFileInfo(target);

		std::ifstream f(target);
		if (f.is_open()) {

			while (getline(f, line)) {
				// # indicates comment
				if (line[0] == '#') continue;

				tokens = Utils::split(line, ' ');

				// Empty line, or line with empty definition such as: 'map_Kd \n'
				if (tokens.size() < 2) continue;

				std::string lineType = tokens.front();

				// NOTE: for the sake of time, ignoring all flags here, and just using the value
				// TODO: This will break if there are flags

				std::string fileName;

				// Start recording information for given material
				if (lineType == "newmtl") {
					// If a new material is encountered while one is being recorded, finish the current one.
					if (current) {
						materials.push_back(current);
					}
					current = new Material(fi.file + '.' + tokens[1]);
					std::cout << "Generating Material " << current->name << std::endl;

				} else if (lineType == "map_Kd") { // DIFFUSE MAP
					fileName = line.substr(7);
					current->map_Kd = new Texture(fi.directory + DIRECTORY_SEPARATOR + fileName);
					current->addFlag(MATERIAL_USE_map_Kd);

				} else if (lineType == "map_Ks"){ // SPEC MAP
					fileName = line.substr(7);
					current->map_Ks = new Texture(fi.directory + DIRECTORY_SEPARATOR + fileName);
					current->addFlag(MATERIAL_USE_map_Ks);
				
				} else if (lineType == "norm") { // NORMAL MAP
					fileName = line.substr(5);
					current->norm = new Texture(fi.directory + DIRECTORY_SEPARATOR + fileName);
					current->addFlag(MATERIAL_USE_map_norm);
				
				} else if (lineType == "illum") { // Illumination
					fileName = line.substr(6);
					current->illum = std::atof(fileName.c_str());
				
				} else if (lineType == "Ka") { // Ambient 
					current->Ka = glm::vec3(std::atof(tokens[1].c_str()), std::atof(tokens[2].c_str()), std::atof(tokens[3].c_str()));
				
				} else if (lineType == "Kd"){ // Diffuse
					current->Kd = glm::vec3(std::atof(tokens[1].c_str()), std::atof(tokens[2].c_str()), std::atof(tokens[3].c_str()));
				
				} else if (lineType == "Ks"){ // Specular color
					current->Ks = glm::vec3(std::atof(tokens[1].c_str()), std::atof(tokens[2].c_str()), std::atof(tokens[3].c_str()));
				
				} else if (lineType == "Ns"){ // Specular exp
					current->Ns = std::atof(line.substr(3).c_str());
				
				} else if (lineType == "Tr"){ // Transparency value, 1 is transparent
					current->Tr = std::atof(line.substr(3).c_str());

				} else if (lineType == "d"){ // Equivalent transparency value, Tr is inverse of d
					current->Tr = 1 - std::atof(line.substr(2).c_str());
				}
			}
		} else {
			std::cerr << "Couldn't open .mtl file: " << target << std::endl;
			return std::vector<Material*>();
		}

		f.close();

		// Save last material
		if (current != nullptr) {
			materials.push_back(current);
		}

		return materials;
	}
}

inline void Material::addFlag(const uint32_t& flag) {
	this->flags |= flag;
}

inline void Material::removeFlag(const uint32_t& flag) {
	this->flags &= ~flag;
}

inline bool Material::checkFlag(const uint32_t& flag) {
	return this->flags & flag;
}

void Material::bind(){
	unsigned int shaderID = this->shader->programID();

	this->shader->bind();
	
	this->shader->setUniform3fv("Ka", this->Ka);
	this->shader->setUniform3fv("Kd", this->Kd);
	this->shader->setUniform3fv("Ks", this->Ks);

	glUniform1f(glad_glGetUniformLocation(this->shader->programID(), "Tr"), this->Tr);

	glUniform1i(glad_glGetUniformLocation(this->shader->programID(), "flags"), this->flags);

	if (this->checkFlag(MATERIAL_USE_map_Kd)) {
		glUniform1i(glGetUniformLocation(shaderID, "map_Kd"), TEXTURE_LOCATION_map_Kd);
		glActiveTexture(GL_TEXTURE0 + TEXTURE_LOCATION_map_Kd);
		glBindTexture(GL_TEXTURE_2D, this->map_Kd->getID());
	}

	if (this->checkFlag(MATERIAL_USE_map_Ks)) {
		glUniform1i(glGetUniformLocation(shaderID, "map_Ks"), TEXTURE_LOCATION_map_Ks);
		glActiveTexture(GL_TEXTURE0 + TEXTURE_LOCATION_map_Ks);
		glBindTexture(GL_TEXTURE_2D, this->map_Ks->getID());
	}

	if (this->checkFlag(MATERIAL_USE_map_norm)) {
		glUniform1i(glGetUniformLocation(shaderID, "norm"), TEXTURE_LOCATION_norm);
		glActiveTexture(GL_TEXTURE0 + TEXTURE_LOCATION_norm);
		glBindTexture(GL_TEXTURE_2D, this->norm->getID());
	}
}

void Material::setMVP(const glm::mat4& mvp) {
	this->shader->setUniformMatrix4fv("MVP", mvp);
}

Material::Material(const std::string& name) {
	this->name = name;
	this->flags = 0;

	this->Ka = glm::vec3(0,0,0);
	this->Kd = glm::vec3(0,0,0);
	this->Ks = glm::vec3(0,0,0);

	this->Ns = 0;
	this->Tr = 0;

	this->map_Kd = nullptr;
	this->map_Ks = nullptr;
	this->norm = nullptr;
}

Material::~Material() {
	delete this->map_Kd;
	delete this->map_Ks;
	delete this->norm;
}
