#include <Renderer.h>

std::string vec3ToString(const glm::vec3& v) {
	char s[32];
	sprintf(s, "[%.3f, %.3f, %.3f]", v[0], v[1], v[2]);

	return std::string(s);
}

void Renderer::registerTexture(const std::string& id, Texture* texture){
    if (this->textures.count(id) == 0){
        this->textures[id] = texture;
		std::cout << "Registered Texture " << id << std::endl;
    } else {
        std::cerr << "Texture " << id << " already registered." << std::endl;
    }
}

void Renderer::registerShader(const std::string& id, Shader* shader){
    if (this->shaders.count(id) == 0){
        this->shaders[id] = shader;
		std::cout << "Registered Shader " << id << std::endl;
    } else {
        std::cerr << "Shader " << id << " already registered." << std::endl;
    }
}

void Renderer::registerMesh(const std::string& id, OBJMesh* mesh){
    if (this->meshes.count(id) == 0){
        this->meshes[id] = mesh;
		std::cout << "Registered Mesh " << id << " (Material " << mesh->getDefaultMaterialName() << ")" << std::endl;
    } else {
        std::cerr << "Mesh " << id << " already registered." << std::endl;
    }
}

void Renderer::registerMaterial(const std::string& id, Material* material){
    if (this->materials.count(id) == 0){
        this->materials[id] = material;
		std::cout << "Registered Material " << id << std::endl;
    } else {
        std::cerr << "Material " << id << " already registered." << std::endl;
    }
}

void Renderer::registerCamera(const std::string& id, Camera* camera){
    if (this->cameras.count(id) == 0){
        this->cameras[id] = camera;
		std::cout << "Registered Camera " << id << std::endl;
    } else {
        std::cerr << "Camera " << id << " already registered." << std::endl;
    }
}

void Renderer::registerSkybox(const std::string& id, Skybox* skybox){
    if (this->skyboxes.count(id) == 0){
        this->skyboxes[id] = skybox;
		std::cout << "Registered Skybox " << id << std::endl;
    } else {
        std::cerr << "Skybox " << id << " already registered." << std::endl;
    }
}

Camera* Renderer::getMainCamera() {
	return this->mainCamera;
}

void Renderer::setMainCamera(const std::string& id){
    try {
        this->mainCamera = this->cameras.at(id);
    }
    catch (const std::out_of_range& oor) {
        std::cerr << "Camera " << id << " not registered" << std::endl;
    }
}

void Renderer::setSkybox(const std::string& id){
    try {
        this->skybox = this->skyboxes.at(id);
    }
    catch (const std::out_of_range& oor) {
        std::cerr << "Skybox " << id << " not registered" << std::endl;
    }
}

void Renderer::drawLine(const glm::vec3& origin, const glm::vec3& end, const glm::vec3& color, bool drawOver) {
	glm::mat4 VP = this->mainCamera->projectionViewMatrix();

	Shader* lineShader = this->shaders.at("line");
	lineShader->bind();

	float line[6];
	memcpy(line, &origin[0], 3 * sizeof(float));
	memcpy(line + 3, &end[0], 3 * sizeof(float));

	glDisable(GL_CULL_FACE);

	if (drawOver) glDisable(GL_DEPTH_TEST);

	GLuint lineVBO, lineVAO;

	glGenBuffers(1, &lineVBO);
	glGenVertexArrays(1, &lineVAO);

	glBindVertexArray(lineVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lineVBO);

	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), line, GL_STREAM_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	lineShader->setUniformMatrix4fv("u_MVP", VP);
	lineShader->setUniform3fv("u_color", color);

	glDrawArrays(GL_LINES, 0, 2);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDeleteBuffers(1, &lineVBO);
	glDeleteVertexArrays(1, &lineVAO);

	glEnable(GL_CULL_FACE);
	if (drawOver) glEnable(GL_DEPTH_TEST);
}

void Renderer::deleteObjects() {
	// TODO: this doesn't quite work
	for (auto k : this->textures) {
		delete k.second;
	}

	for (auto k : this->shaders) {
		delete k.second;
	}

	for (auto k : this->meshes) {
		delete k.second;
	}

	for (auto k : this->materials) {
		delete k.second;
	}

	for (auto k : this->cameras) {
		delete k.second;
	} 

	for (auto k : this->objects){
		delete k.second;
	}
}

Renderer::Renderer(GLFWwindow* window) {
	std::cout << "Initializing renderer..." << std::endl << std::endl;

	this->window = window;

	this->resetData();

	// Get line width parameters from opengl
	float lineWidthBounds[2];
	glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, lineWidthBounds);
	this->lineWidthMax = lineWidthBounds[1];
	glLineWidth(lineWidthBounds[0]);

	std::cout << "Renderer initialized." << std::endl;
}

void Renderer::resetData() {
	// Create and register default assets.

	// Default camera is perspective camera
	Camera* defaultCam = new Camera(45.0f, 1, 0, 2000);

	Shader* defaultShader = new Shader("default", "default");

	Shader* diffuseShader = new Shader(
		std::string(WorldState.projectRoot) + "/resources/shaders/diffuse.vert", 
		std::string(WorldState.projectRoot) + "/resources/shaders/diffuse.frag"
	);

	Shader* lineShader = new Shader(
		std::string(WorldState.projectRoot) + "/resources/shaders/line.vert", 
		std::string(WorldState.projectRoot) + "/resources/shaders/line.frag"
	);

	Shader* skyboxShader = new Shader(
		std::string(WorldState.projectRoot) + "/resources/shaders/skybox.vert", 
		std::string(WorldState.projectRoot) + "/resources/shaders/skybox.frag"
	);

	Texture* defaultTexture = new Texture("default");

	Material* defaultMaterial = new Material("default");
	defaultMaterial->shader = defaultShader;

	std::vector<std::string> skyboxFaces = {
		std::string(WorldState.projectRoot) + "/resources/textures/default_skybox+X.png",
		std::string(WorldState.projectRoot) + "/resources/textures/default_skybox-X.png",
		std::string(WorldState.projectRoot) + "/resources/textures/default_skybox+Y.png",
		std::string(WorldState.projectRoot) + "/resources/textures/default_skybox-Y.png",
		std::string(WorldState.projectRoot) + "/resources/textures/default_skybox+Z.png",
		std::string(WorldState.projectRoot) + "/resources/textures/default_skybox-Z.png",
	};

	Skybox* defaultSkybox = new Skybox(skyboxFaces);

	this->registerShader("diffuse", diffuseShader);
	this->registerShader("line", lineShader);
	this->registerShader("skybox", skyboxShader);
	this->registerShader("default", defaultShader);

	this->registerCamera("default", defaultCam);
	this->registerTexture("default", defaultTexture);
	this->registerMaterial("default", defaultMaterial);
	this->registerSkybox("default", defaultSkybox);

	this->setMainCamera("default");
	this->setSkybox("default");

	this->numOfLights = 0;

	this->frameCount = 0;
}

Renderer::~Renderer() {
	this->deleteObjects();
}

void Renderer::loadScene(const std::string& target) {
	std::ifstream f;
	f.open(target);

	if (!f.is_open()) {
		std::cerr << "Could not open target scene for loading: " << target << std::endl;
		return;
	}

	this->loading = true;

	Utils::StopWatch timer;

	std::cout << std::endl << "Loading Scene " << target << "..." << std::endl;

	this->scene.name = Utils::getFileInfo(target).file;
	this->scene.path = target;
	this->scene.files = std::vector<std::string>();

	int lines = Utils::getLines(target);
	std::string line;
	this->progress = 0.0f;
	float progressStep = 1.0f / lines;

	timer.start();

	while (std::getline(f, line)) {
		this->progress += progressStep;

		if (line == "" || line[0] == '#') continue;

		std::vector<std::string> tokens = Utils::split(line, ' ');
		std::string lineType = tokens[0];

		// Name of scene
		if (lineType == "name") {
			int nameLength = line.size() - 5; // "name " is 5 chars
			this->scene.name = line.substr(5, nameLength);
		}
		// Light to be used in scene
		else if (lineType == "light") {
			if (this->numOfLights >= MAX_LIGHT_COUNT) {
				std::cerr << "Too many lights attempting to be created for scene. (Max light count is " << MAX_LIGHT_COUNT << ")" << std::endl;
				continue;
			}

			std::string word;

			int k = 1;

			Light light = {tokens[k++] == "directional" ? LightType::DIRECTIONAL : LightType::POINT};

			while (k < tokens.size()){
				word = tokens[k++];	

				if (word == "-p" || word == "-d" || word == "-v"){
					light.setPointDirection({ std::atof(tokens[k++].c_str()), std::atof(tokens[k++].c_str()), std::atof(tokens[k++].c_str()) });
				}

				if (word == "-c"){
					light.setColor({ std::atof(tokens[k++].c_str()), std::atof(tokens[k++].c_str()), std::atof(tokens[k++].c_str()) });
				}

				if (word == "-i"){
					light.setIntensity(std::atof(tokens[k++].c_str()));
				}

				if (word == "-i" || word == "-f"){
					light.setK(std::atof(tokens[k].c_str()));
				}
			}

			std::cout << "Created new " << tokens[1] << " light." << std::endl;

			this->lights[this->numOfLights++] = light;
		}
		else if (lineType == "ambient") {
			::WorldState.ambientLight[0] = std::atof(tokens[1].c_str());
			::WorldState.ambientLight[1] = std::atof(tokens[2].c_str());
			::WorldState.ambientLight[2] = std::atof(tokens[3].c_str());
		}
		// Line is just path to some file to be included in the scene
		else { 
			std::string fullFileName = "";
			fullFileName += WorldState.trackDataRoot ;
			fullFileName += '/' ;
			fullFileName += line;

			std::cout << "Loading " << fullFileName << std::endl;

			Utils::FileInfo fi = Utils::getFileInfo(fullFileName);

			if (fi.extension == "mtl"){ // Load mtl library file
				this->scene.files.push_back(fullFileName);
				this->loadMaterialLibrary(fullFileName);
			} else if (fi.extension == "obj") { // Load obj file
				this->scene.files.push_back(fullFileName);
				this->loadOBJ(fullFileName);
			} else if (fi.extension == "vert") { // Load vertex shader
				std::cout << "Loading vertex shader...";
			} else if (fi.extension == "frag") { // Load fragment shader
				std::cout << "Loading fragment shader...";
			} else {
				std::cerr << "Unsupported file type " << fi.extension << " for file " << fullFileName << std::endl;
			}
		}
	}

	// All necessary files loaded, now link all resources into Objects to be rendered.

	for (auto p : this->meshes){
		Object* o = this->newObject(p.first);

		o->mesh = p.second;
	}

	// Now that all necessary Objects have been generated with their meshes, link parents and materials
	Object* o;
	for (auto p : this->objects){
		o = p.second;
		std::string targetParent = o->mesh->getDefaultParentName();
		std::string targetMaterial = o->mesh->getDefaultMaterialName();

		if (targetParent != "") {
			try {
				o->transform->setParent(this->objects.at(targetParent)->transform);
			}
			catch (const std::out_of_range & oor) {
				std::cerr << "Can't find parent " << "<" << targetParent << ">" << " for object " << p.first << std::endl;
			}
		}

		if (targetMaterial != ""){
			try {
				o->material = this->materials.at(targetMaterial);
			} catch (const std::out_of_range& oor){
				std::cerr << "Can't find material " << "<" << targetMaterial << "> for object " << p.first << std::endl;
			}
		} else {
			o->material = this->materials.at("default");
		}
	}

	std::cout << "Finished loading scene " << this->scene.name << " (" << timer.lap_s() << ")" << std::endl;

	this->loading = false;
}

void Renderer::loadMaterialLibrary(const std::string& target) {
	std::cout << std::endl << "Loading material library " << target << std::endl << std::endl;

	this->loading = true;

	Utils::FileInfo fi = Utils::getFileInfo(target);

	std::map<std::string, Material*> materials = MTL::load(target);

	for (auto p : materials) {
		p.second->shader = this->shaders.at("diffuse");
		this->registerMaterial(p.first, p.second);
	}

	this->loading = false;
}

void Renderer::loadOBJ(const std::string& target){
	std::cout << std::endl << "Loading OBJ file " << target << std::endl << std::endl;

	this->loading = true;

	std::vector<OBJMesh*> meshes = OBJ::load(target);

	for (OBJMesh* mesh : meshes){
		this->registerMesh(mesh->getMeshName(), mesh);
	}

	this->loading = false;
}

Object* Renderer::newObject(const std::string& name) {
	if (this->objects.count(name) != 0) {
		std::cerr << "Object " << name << " already exists." << std::endl;
		return nullptr;
	}

	Object* object = new Object(name);
	
	object->transform = new Transform();
	object->material = this->materials.at("default");

	this->objects[name] = object;

	return object;
}

Object* Renderer::getObject(const std::string& name) {
	if (this->objects.count(name) != 1) {
		std::cerr << "Object " << name << " is not registered with renderer.";
		return nullptr;
	}

	return this->objects.at(name);
}

void Renderer::tick(const double& dTime) {
	// NOTE: translateion rotation controls are just for testings, not to be included in future builds

	++this->frameCount;

	float translation[3] = { 0,0,0 };
	float rotation[2] = { 0,0 };

	float translateSpeed = dTime * 100;
	float rotationSpeed = dTime * 1.0f;

	// TRANSLATION
	if (glfwGetKey(this->window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(this->window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		translation[1] = translateSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		translation[0] = -translateSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		translation[1] = -translateSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		translation[0] = translateSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		translation[2] = translateSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		translation[2] = -translateSpeed;
	}

	// ROTATION 
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		rotation[1] = rotationSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		rotation[0] = rotationSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		rotation[1] = -rotationSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		rotation[0] = -rotationSpeed;
	}

	// Get gpu-friendly matrix representation for lights 
	for (int i = 0; i < this->numOfLights; i++) {
		*(this->lightMatrices + i) = (this->lights + i)->getMatrix();
	}

	// Calcuate new MVP for camera on this frame
	Transform* camTransform = this->mainCamera->transform;

	glm::vec3 worldUp = glm::inverse(camTransform->getMatrix()) * glm::vec4(0, 1, 0, 0);

	camTransform->rotate(rotation[0], worldUp);
	camTransform->rotate(-rotation[1], glm::vec3(1,0,0));

	glm::vec3 dx, dy, dz;
	if (abs(translation[0]) > 0.01) {
		dx = -1 * translation[0] * camTransform->right();
		dx.y = 0;
		dx = glm::normalize(dx);
		dx *= translateSpeed;
	}
	else {
		dx = { 0,0,0 };
	}

	dy = translation[2] * glm::vec3(0, 1, 0);

	if (abs(translation[1]) > 0.01) {
		dz = -1.0f * translation[1] * camTransform->forward();
		dz.y = 0;
		dz = normalize(dz);
		dz *= translateSpeed;
	}
	else {
		dz = { 0,0,0 };
	}

	camTransform->translate(dx + dy + dz);

	glm::mat4 VP = this->mainCamera->projectionViewMatrix();

	std::map<std::string, std::vector<Object*>> materialMapping;

	Object* o;
	Material* m;
	Shader* s;
	std::string materialName;

	for (std::pair<std::string, Object*> p : this->objects){
		o = p.second;
		materialName = o->material->name;	

		if (materialMapping.count(materialName) == 0){
			materialMapping[materialName] = std::vector<Object*>();
		}

		materialMapping[materialName].push_back(o);
	}

	for (std::pair<std::string, std::vector<Object*>> p : materialMapping){
		m = this->materials.at(p.first);
		s = m->shader;

		m->bind();

		glUniform3fv(glad_glGetUniformLocation(s->programID(), "Ka"), 1, &WorldState.ambientLight[0]);

		s->setUniformMatrix4fv("MV", VP);

		s->setLights(this->numOfLights, this->lightMatrices);

		for (Object* o : p.second){
			s->setUniformMatrix4fv("MVP", VP * o->transform->getMatrix());
			o->mesh->draw();
		}
	}
	
	// Draw coordinate axes (for testing)
	// TODO: remove
	// this->drawLine(glm::vec3(), glm::vec3(10, 0, 0), glm::vec3(1, 0, 0));
	// this->drawLine(glm::vec3(), glm::vec3(0, 10, 0), glm::vec3(0, 1, 0));
	// this->drawLine(glm::vec3(), glm::vec3(0, 0, 10), glm::vec3(0, 0, 1));

	if (this->skybox != nullptr){
		glDisable(GL_CULL_FACE);

		Shader* skyboxShader = this->shaders.at("skybox");

		skyboxShader->bind();
		skyboxShader->setUniformMatrix4fv("MVP", VP);

		this->skybox->draw();

		glEnable(GL_CULL_FACE);
	}

	/*
		skybox position = mainCamera.position
		skybox rotation = 0v

		draw skybox*
		*set gl_Position in shader = pos with z=1

	*/

}
 
void Renderer::setLineWidth(const float& w) {
	if (1 <= w <= this->lineWidthMax) glLineWidth(w);
}
